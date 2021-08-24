/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "globl.h"
#include "ui.h"
#include "pack.h"
#include "sim.h"
#include "atria.h"

int                   mapfd;
void                * map_raw;
struct atria_header * map_hdr;
uint16_t            * map_exe;

int                   bmapfd;
uint16_t            * bmap;

uint16_t             reg_sid;
struct exe_partition exe;
struct op_partition  op1, op2;

char opr[120], in_mod[120], out_mod[120];

static int sim_recv_new ( void );
static int sim_init_image ( void );
static void sim_update_allocations ( void );
static void sim_display_srf ( uint16_t sid );

static void sim_auto_command ( int sock );

static int  sim_dispatch_ticket ( int sock, tag * ticket );
static int  sim_load_overlay ( int sock, uint16_t unit );
static int  sim_register_srf
( struct atria_srf * srf, struct atria_sisr * sisr, uint8_t intr );
static struct atria_srf * sim_pull_srf ( uint16_t sid );

static int  sim_load_master ( int sock );
static int  sim_exec_commission
( int sock, tag * ticket, struct atria_srf * srf  );
static int  sim_exec_gen_rel  ( int sock, tag * ticket );
static int  sim_exec_gen_mod  ( int sock, tag * ticket );
static int  sim_exec_gen_imm  ( int sock, tag * ticket );
static int  sim_exec_gen_jmp  ( int sock, tag * ticket );
static int  sim_exec_gen_spec
( int sock, tag * ticket, struct atria_srf * srf );

static void sim_dump_elf ( void );

extern void sim_base ( void )
{
     int smax, smay;
     WINDOW * dbox;

     /* This doesn't "stay running" in the background */
     /* every time is a new set-up */
     mapfd = -1;

     memset ( &map_raw,  0, sizeof map_raw );
     memset ( &map_hdr,  0, sizeof map_hdr );
     memset ( &map_exe,  0, sizeof map_exe );
     
     wclear ( simscr );
     UI_fwin ( simscr,
               "ATRIA Simulator (atmega328p)",
               "" );
     touchwin ( simscr );

     actscr = simscr;
     wrefresh ( actscr );

     /* We try to set up as much stuff as possible before */
     /* we call it quits */
     getmaxyx ( actscr, smay, smax );
     smay--; smax--;

     /* We need to have an image file open already, or we */
     /* make a new one */
     if ( !elfmain )
          sim_init_image ( );

     if ( !elfmain )
     {
          UI_msgbox ( "ERROR",
                      "Unable to initialize image",
                      "Press any key to abort" );
          return;
     }

     /* Get the file descriptor for the stream */
     mapfd = fileno ( elfmain );
     
     if ( mapfd < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Cannot get file descriptor",
                           "Press any key to return" );
          return;
     }

     /* Now we create a memory map for the file */
     map_raw = mmap ( 0, IMG_SIZE,
                      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NOCORE,
                      mapfd, 0 );

     if ( map_raw == MAP_FAILED )
     {
          UI_msgbox ( "ERROR",
                      "Unable to create memory map",
                      "Press any key to return" );
          return;
     }

     /* Now we verify that this is infact an ATRIA image  */
     /* Set up pointers */
     map_hdr = (struct atria_header *)map_raw;
     map_exe = (uint16_t *)&map_hdr[1];

     /* Verify header */
     if ( strncmp ( map_hdr->magic, AMAGIC, 16 ) != 0 )
     {
          UI_msgbox ( "ERROR",
                      "Image header incorrect",
                      "Press any key to abort" );

          goto quit;
     }

     /* If this is the first one, we need to "receive" on it */
     if ( !map_hdr->srf_alpha )
          if ( (sim_recv_new ( ) < 0) || (Auto.at) )
               goto quit;
               

     /* So here we are, the "main" menu, we display the */
     /* contents of an SRF and allow editing */
     sim_display_srf ( 0 );      /* opsec */
     
quit:
     fsync ( mapfd );
     munmap ( map_raw, IMG_SIZE );
     close ( mapfd );

     if ( Auto.at )
          exit ( 0 );
     
     return;

}


static int sim_recv_new ( void )
{
     int sock, map, i;
     struct sockaddr_un addr;
     struct iovec msgvec;
     struct msghdr msg;
     struct cmsghdr * ctl;

     union
     {
          char           buf[ CMSG_SPACE(sizeof (int)) ];
          struct cmsghdr align;
     } mapxfer;

     char sock_path[120];
     
     tag tic_in, tic_out;
     
     int smax, smay;
     WINDOW * dbox;



     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );
     memset ( &mapxfer,  0, sizeof mapxfer );

     memset ( sock_path, 0, 120            );
     memset ( &tic_in,   0, sizeof tic_in  );
     memset ( &tic_out,  0, sizeof tic_out );

     memset ( &reg_sid,  0, sizeof reg_sid );
     memset ( &exe,      0, sizeof exe     );
     memset ( &op1,      0, sizeof op1     );
     memset ( &op2,      0, sizeof op2     );
     memset ( opr,       0, 120            );
     memset ( in_mod,    0, 120            );
     memset ( out_mod,   0, 120            );

     wclear ( actscr );
     UI_fwin ( actscr,
               "ATRIA Simulator (atmega328p)",
               "" );
     touchwin ( simscr );

     wrefresh ( actscr );

     /* We try to set up as much stuff as possible before */
     /* we call it quits */
     getmaxyx ( actscr, smay, smax );
     smay--; smax--;


     
     /* Ok now we need to ask the user to provide the name */
     /* pipe to connect to                                 */

     sock = socket ( PF_UNIX, SOCK_DGRAM, 0 );
     if ( sock < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Cannot open UNIX Socket",
                      "Press any key to return" );
          close  ( sock );
          return ( -1 );
     }

     
     while ( 1 )
     {
          memset ( &addr, 0, sizeof addr );

          
          if (!Auto.cpoint)
          {
               dbox = UI_cwin ( 5, 64,
                                "Enter UNIX Socket to listen on",
                                "Press ENTER to Accept" );
               wattrset ( dbox, A_REVERSE | A_UNDERLINE );
               for ( i = 0; i < 60; i++ )
                    mvwaddch ( dbox, 2, (i+2), ' ' );
               UI_prompt ( dbox, 2, 2, sock_path, 60 );
               delwin ( dbox );
          }
          else
          {
               strncpy ( sock_path, Auto.cpoint, 120 );
               sock_path[119] = '\0';
          }

          if ( !sock_path[0] )
               return ( -1 );
          
          /* Attempt to bind */
          addr.sun_family = AF_UNIX;
          strncpy ( addr.sun_path, sock_path, 103 );
          addr.sun_len = SUN_LEN(&addr);

          i = bind ( sock, (struct sockaddr *)&addr, sizeof addr );

          if ( i == 0 )
               break;
          
          UI_msgbox ( "ERROR",
                      "Cannot bind specified socket.",
                      "Press any key to retry" );
     }


     /* Now wait for the incoming registration request */
     wattrset ( actscr, A_REVERSE );
     mvwaddstr ( actscr, smay, 1, "Waiting for registration request..." );
     wmove ( actscr, smay, smax );
     wattrset ( actscr, A_NORMAL );
     touchwin ( actscr );
     wrefresh ( actscr );

     msgvec.iov_base = &tic_in;
     msgvec.iov_len  = sizeof tic_in;

     msg.msg_iov        = &msgvec;
     msg.msg_iovlen     = 1;
     msg.msg_control    = &mapxfer;
     msg.msg_controllen = sizeof mapxfer;

     ctl = CMSG_FIRSTHDR(&msg);

     i = recvmsg ( sock, &msg, 0 );

     if ( i <= 0 )
     {
          UI_msgbox ( "ERROR",
                      "Received corrupt handshake",
                      "Press any key to abort" );
          unlink ( addr.sun_path );
          close ( sock );
          return ( -1 );
     }

     if ( !strncmp ( ((unsigned char *)(&tic_in)), "ELF", 3 ) )
     {
          /* Explicit elf output command */
          sim_dump_elf ( );
          unlink ( addr.sun_path );
          close ( sock );
          fsync ( mapfd );
          exit ( 0 );
     }
     else if ( !strncmp ( ((unsigned char *)(&tic_in)), "CLOSE", 5 ) )
     {
          unlink ( addr.sun_path );
          close ( sock );
          fsync ( mapfd );
          exit ( 0 );
     }
     else if ( ctl->cmsg_type != SCM_RIGHTS )
     {
          
          UI_msgbox ( "ERROR",
                      "No case board transfer",
                      "Press any key to abort" );

          unlink ( addr.sun_path );
          close ( sock );
          return ( -1 );
     }

     /* Now this will be a new socket for us to use */
     unlink ( addr.sun_path );
     close  ( sock );
     sock = *((int *)CMSG_DATA(ctl));

     /* Now we are finally ready to recive the first    */
     /* ticket on the new pipe. This first ticket will  */
     /* also provide the case board shared memory map   */

     memset ( &tic_in,   0, sizeof tic_in  );
     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );
     memset ( &mapxfer,  0, sizeof mapxfer );

     msgvec.iov_base = &tic_in;
     msgvec.iov_len  = sizeof tic_in;

     msg.msg_iov        = &msgvec;
     msg.msg_iovlen     = 1;
     msg.msg_control    = &mapxfer;
     msg.msg_controllen = sizeof mapxfer;

     ctl = CMSG_FIRSTHDR(&msg);

     i = recvmsg ( sock, &msg, 0 );

     if ( i < sizeof tic_in )
     {
          UI_msgbox ( "ERROR",
                      "Received corrupt ticket",
                      "Press any key to abort" );
          close ( sock );
          return ( -1 );
     }

     if ( ctl->cmsg_type != SCM_RIGHTS )
     {
          UI_msgbox ( "ERROR",
                      "No case board transfer",
                      "Press any key to abort" );

          close ( sock );
          return ( -1 );
     }
     
     bmapfd = *((int *)CMSG_DATA(ctl));
     bmap   = (uint16_t *)mmap ( NULL, sizeof (uint16_t) * BOARD_SIZE,
                                 PROT_READ | PROT_WRITE,
                                 MAP_NOCORE | MAP_NOSYNC | MAP_SHARED,
                                 bmapfd, 0 );

     if ( ((void *)bmap) == MAP_FAILED )
     {
          UI_msgbox ( "ERROR",
                      "Could not instantiate shared memory",
                      "Press any key to abort" );
          close ( sock );
          return ( -1 );
     }

     /* OK, now we are requiring the first ticket be a NEW_REG  */
     /* if it is ok, and we accept this action, we return the   */
     /* ticket, as is, with a new socket pair to communicate on */
     if ( tic_in.s != XST_MSG )
     {
          UI_msgbox ( "ERROR",
                      "Received invalid ticket",
                      "Press any key to abort" );
          close ( sock );
          return ( -1 );
     }

     if ( tic_in.t.msg.ref != NEW_REG )
     {
          UI_msgbox ( "ERROR",
                      "Received invalid ticket",
                      "Press any key to abort" );
          close ( sock );
          return ( -1 );
     }
     
     /* Ok, cool! Set our SID */
     reg_sid = tic_in.t.msg.arg.a16[0];



     /* Now we need to check if this is a fresh image   */
     /* We know that if SRF is 0. In that case, we need */
     /* to specially initialize the image with opsec    */
     if ( !map_hdr->srf_alpha )
     {
          if ( reg_sid )
          {
               /* sid must be 0000! */
               UI_msgbox ( "ERROR",
                           "OPSEC% must be registered first!",
                           "Press any key to abort" );
               close ( sock );
               return ( -1 );
          }

          /* Set up zero-offsets */
          map_hdr->img_ext = EXE_MIN;
          map_hdr->op_ext = OP_MIN;
          
          exe.off = map_hdr->img_ext;
          op1.off = map_hdr->op_ext;
          op2.off = op1.off;

          exe.cursor = map_exe;
     }
     else
     {
          if ( !sim_pull_srf )
          {
               UI_msgbox ( "ERROR",
                           "SID is already registered",
                           "Press any key to select other" );
               close ( sock );
               return ( -1 );

          }
          
          /* set up to the extents */
          exe.off = map_hdr->img_ext + 1;
          op1.off = map_hdr->op_ext + 1;
          op2.off = op1.off;

          exe.cursor = &map_exe[exe.off];
          
     }
          

     if ( !Auto.at )
          sim_update_allocations ( );
          
     if ( sim_load_master ( sock ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Registration failed",
                      "Press any key to abort" );
          
          close ( sock );
          return ( -1 );
     }

     if ( !Auto.at )
          sim_update_allocations ( );
     
     close ( sock );

     return ( 0 );
}


static int sim_init_image ( void )
{
     int i;
     WINDOW * dbox;

     struct atria_header newhdr;
     char fname[120];

     memset ( &newhdr, 0, sizeof newhdr );

     if (!Auto.img)
     {
          dbox = UI_cwin ( 5, 64,
                           "Enter New Image File",
                           "Press ENTER to Accept" );
          wattrset ( dbox, A_REVERSE | A_UNDERLINE );
          for ( i = 0; i < 60; i++ )
               mvwaddch ( dbox, 2, (i+2), ' ' );
          UI_prompt ( dbox, 2, 2, fname, 60 );
          delwin ( dbox );
     }
     else
     {
          strncpy ( fname, Auto.img, 120 );
          fname[119] = '\0';
     }

     if ( !fname[0] )
          return ( -1 );
     
     mapfd = open ( fname, O_CREAT | O_RDWR | O_EXCL );

     if ( mapfd < 0 )
     {
          strerror_r ( errno, fname, 120 );
          UI_msgbox ( "Unable to Open",
                      fname,
                      "Press any key to return" );
          return ( -1 );
     }

     /* Set size */
     i = ftruncate ( mapfd, IMG_SIZE );

     if ( i < 0 )
     {
          strerror_r ( errno, fname, 120 );
          UI_msgbox ( "Unable to Truncate",
                      fname,
                      "Press any key to return" );
          close ( mapfd );
          mapfd = -1;
          return ( -1 );
     }

     i = fchmod ( mapfd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
     if ( 1 < 0 )
     {
          strerror_r ( errno, fname, 120 );
          UI_msgbox ( "Unable to set permissions",
                      fname,
                      "Press any key to return" );
          close ( mapfd );
          elfmain = NULL;
          mapfd = -1;
          return ( -1 );
     }

     /* OK, set up the regular file descriptor so we can */
     /* use the HEX viewer if we want... */
     elfmain = fdopen ( mapfd, "r+" );

     if ( !elfmain )
     {
          strerror_r ( errno, fname, 120 );
          UI_msgbox ( "Unable to Stream",
                      fname,
                      "Press any key to return" );
          close ( mapfd );
          elfmain = NULL;
          mapfd = -1;
          return ( -1 );
     }

     /* Finally, write out a new header */
     strncpy ( newhdr.magic, AMAGIC, 16 );
     i = write ( mapfd, &newhdr, sizeof newhdr );

     if ( i != sizeof newhdr )
     {
          strerror_r ( errno, fname, 120 );
          UI_msgbox ( "Unable to Write Header",
                      fname,
                      "Press any key to return" );
          close ( mapfd );
          elfmain = NULL;
          mapfd = -1;
          return ( -1 );
     }

     fsync ( mapfd );
     
}


static void sim_update_allocations ( void )
{
     WINDOW * databox;

     if (Auto.at)
          return;

     databox = derwin ( actscr, 8, 35, 3, 2 );
     if ( !databox )
          err_nomem ( );

     wclear ( databox );

     wattrset ( databox, A_NORMAL );
     box ( databox, 0, 0 );
     mvwaddstr ( databox, 2, 2, "SID" );
     mvwaddstr ( databox, 3, 2, "EXE" );
     mvwaddstr ( databox, 4, 2, "OP1" );
     mvwaddstr ( databox, 5, 2, "OP2" );

     wattrset ( databox, A_BOLD );
     wmove ( databox, 2, 8 );
     wprintw ( databox, "%04X%%", reg_sid );

     wmove ( databox, 3, 8 );
     wprintw ( databox, "%08X + %08X", exe.off, exe.ext );

     wmove ( databox, 4, 8 );
     wprintw ( databox, "%08X + %08X", op1.off, op1.ext );

     wmove ( databox, 5, 8 );
     wprintw ( databox, "%08X + %08X", op2.off, op2.ext );

     wrefresh ( databox );
     delwin ( databox );

     databox = derwin ( actscr, 8, 73, 12, 2 );
     if ( !databox )
          err_nomem ( );

     wclear ( databox );
     wattrset ( databox, A_NORMAL );
     box ( databox, 0, 0 );

     mvwaddstr ( databox, 2, 2, "Operation" );
     mvwaddstr ( databox, 3, 2, "Target" );
     mvwaddstr ( databox, 4, 2, "Original" );
     mvwaddstr ( databox, 5, 2, "Modified" );

     wattrset ( databox, A_BOLD );
     mvwaddstr ( databox, 2, 14, opr );

     wmove ( databox, 3, 14 );
     wprintw ( databox, "RAW:%08x IMG:%08x EXE:%08x",
               (uint32_t)(exe.cursor - (uint16_t *)map_raw),
               (uint32_t)(exe.cursor - map_exe),
               (uint32_t)(exe.cursor - map_exe) - exe.off );

     mvwaddstr ( databox, 4, 14, in_mod );
     mvwaddstr ( databox, 5, 14, out_mod );
     
     wrefresh ( databox );
     delwin   ( databox );


     /* Header data */
     databox = derwin ( actscr, 8, 35, 3, 40 );
     if ( !databox )
          err_nomem ( );

     wclear ( databox );
     wattrset ( databox, A_NORMAL );
     box ( databox, 0, 0 );

     mvwaddstr ( databox, 2, 2, "Magic"          );
     mvwaddstr ( databox, 3, 2, "SRF"            );
     mvwaddstr ( databox, 4, 2, "SISR"           );
     mvwaddstr ( databox, 5, 2, "Offset"         );

     wattrset ( databox, A_BOLD );
     wmove ( databox, 2, 10 );
     mvwaddstr ( databox, 2, 10, map_hdr->magic );

     wmove ( databox, 3, 10 );
     wprintw ( databox, "%08X", map_hdr->srf_alpha );

     wmove ( databox, 4, 10 );
     wprintw ( databox, "%08X", map_hdr->sisr_top );

     wmove ( databox, 5, 10 );
     wprintw ( databox, "%08X", exe.off );

     wrefresh ( databox );
     delwin   ( databox );

     return;
     
}


static void sim_display_srf ( uint16_t sid )
{
     int smay, smax, may, max, inp, i, j;
     WINDOW *fwin;

     char sid_sel[5];

     struct atria_srf * srf;

     /* If auto, it's one shot and go */
     if (Auto.at)
     {
          sim_recv_new ( );
          return;
     }
          
          

     getmaxyx ( actscr, smay, smax );
     smay--; smax--;

     fwin = derwin ( actscr, 16, 60, 3, (smax/2) - 30 );
     if ( !fwin )
          err_nomem ( );
     getmaxyx ( fwin, may, max );
     may--; max--;

reset:
     wclear ( actscr );
     UI_fwin ( actscr,
               "ATRIA Simulator (atmega328p)",
               "" );

     wattrset ( actscr, A_REVERSE );
     mvwaddstr ( actscr, smay, 20,
                 "(w) Write ELF (s) Select SID (n) Load New (q) Quit" );
     wattrset ( actscr, A_NORMAL );
     wrefresh ( actscr );


     wmove ( actscr, smay, 1 );
     wattrset ( actscr, A_STANDOUT | A_BOLD );
     wprintw ( actscr, "%04X%%", sid );
     wattrset ( actscr, A_NORMAL );
     wrefresh ( actscr );
     
     /* Seek srf */
     srf = sim_pull_srf ( sid );
     if ( !srf )
     {
          UI_msgbox ( "Not Registered",
                      "SID is not registered",
                      "Press any key to select other" );
          goto select_sid;
     }


     wattrset ( fwin, A_NORMAL );
     wclear ( fwin );
     
     box ( fwin, 0, 0 );
     wattrset ( fwin, A_REVERSE );

     for ( i = 1; i < max; i++ )
          mvwaddch ( fwin, 1, i, ' ' );
     
     UI_cputs ( fwin, 1, "Secretariat Registration File" );
     wattrset ( fwin, A_BOLD );

     mvwaddstr ( fwin, 3, 2, "EXE" );
     mvwaddstr ( fwin, 4, 2, "OP1" );
     mvwaddstr ( fwin, 5, 2, "OP2" );

     wattron ( fwin, A_UNDERLINE );
     mvwaddstr ( fwin, 7, 2, "Operational Certificate" );
     wattrset ( fwin, A_BOLD );

     mvwaddstr ( fwin, 9,  2, "sid" );
     mvwaddstr ( fwin, 10, 2, "dd" );
     mvwaddstr ( fwin, 11, 2, "bpri" );
     mvwaddstr ( fwin, 12, 2, "dsec" );
     mvwaddstr ( fwin, 13, 2, "usec" );


     wattrset ( fwin, A_REVERSE );
     for ( i = 7; i < 14; i++ )
          for ( j = 30; j < 57; j++ )
               mvwaddch ( fwin, i, j, ' ' );
     wattron ( fwin, A_BOLD | A_STANDOUT );
     mvwaddstr ( fwin, 7, 31, "Image Properties" );
     wattrset ( fwin, A_REVERSE );
     mvwaddstr ( fwin, 9, 31,  "RAW Offset" );
     mvwaddstr ( fwin, 10, 31, "SRF Alpha " );
     mvwaddstr ( fwin, 11, 31, "SISR      " );
     mvwaddstr ( fwin, 12, 31, "IMG Extnt"  );

     /* Fill in the data */
     wattrset ( fwin, A_BOLD );
     wmove ( fwin, 3, 10 );
     wprintw
          ( fwin, "0x%08x - 0x%08x",
            (uint32_t)(*(uint16_t *)srf->text.top + (srf->text.top[2] << 16)),
            (uint32_t)(*(uint16_t *)srf->text.bot + (srf->text.top[2] << 16)) );
     
     wmove ( fwin, 4, 10 );
     wprintw
          ( fwin, "0x%08x - 0x%08x <- EXE:0x%08x",
            srf->data.lda,
            srf->data.siz > 0 ? ( (srf->data.lda + srf->data.siz) - 1 ) : 0,
            (uint32_t)(*(uint16_t *)srf->data.ofs + (srf->data.ofs[2] << 16)));

     wmove ( fwin, 5, 10 );
     wprintw ( fwin, "0x%08x - 0x%08x", srf->bss.lda,
               srf->bss.siz > 0 ? ( (srf->bss.lda + srf->bss.siz) - 1 ) : 0 );

     /* soc */

     wattroff ( fwin, A_UNDERLINE );
     wmove ( fwin, 9, 10 );
     wprintw ( fwin, "%04X%%", srf->soc.sid );

     wattrset ( fwin, A_NORMAL );
     wmove ( fwin, 10, 10 );
     wprintw ( fwin, "0x%08x", srf->soc.dd );

     wmove ( fwin, 11, 10 );
     wprintw ( fwin, "0x%04x", srf->soc.bpri );

     wmove ( fwin, 12, 10 );
     wprintw ( fwin, "0x%04x", srf->soc.dsec );

     wmove ( fwin, 13, 10 );
     wprintw ( fwin, "0x%04x", srf->soc.usec );

     wattrset ( fwin, A_REVERSE );
     wmove ( fwin, 9, 45 );
     wprintw ( fwin, "0x%08x",
               sizeof (struct atria_header) / 2 );

     wmove ( fwin, 10, 45 );
     wprintw ( fwin, "0x%08x", map_hdr->srf_alpha );

     wmove ( fwin, 11, 45 );
     wprintw ( fwin, "0x%08x", map_hdr->sisr_top );
     
     wmove ( fwin, 12, 45 );
     wprintw ( fwin, "0x%08x", map_hdr->img_ext );

     wmove ( actscr, smay, smax );
     wrefresh ( fwin );



poll_key:
     
     flushinp ( );
     inp = getch ( );

     switch ( inp )
     {
          case 'n':
          case 'N':
               sim_recv_new ( );
               sid = reg_sid;
               goto reset;
               
          case 'q':
          case 'Q':
               delwin ( fwin );
               return;
          
          case 's':
          case 'S':
               /* Select */
               goto select_sid;

          case 'w':
          case 'W':
               sim_dump_elf ( );
               goto reset;

          default:
               goto poll_key;
     }



select_sid:

     wattrset ( actscr, A_UNDERLINE );
     snprintf ( sid_sel, 4, "%04x", sid );
     UI_prompt ( actscr, smay, 1, sid_sel, 4 );
     sid = (uint16_t)strtol ( sid_sel, NULL, 16 );

     goto reset;
     
}


static int  sim_dispatch_ticket ( int sock, tag * ticket )
{
     int retval;
     
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     msgvec.iov_base = ticket;
     msgvec.iov_len  = sizeof (tag);

     msg.msg_iov     = &msgvec;
     msg.msg_iovlen  = 1;

     retval = sendmsg ( sock, &msg, 0 );
     if ( retval != sizeof (tag) )
     {
          UI_msgbox ( "ERROR",
                      "Ticket transmission failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Wait for return */
     /* (Unless we sent a kill) */
     if ( ticket->s == 0xffff )
          return ( 0 );
     
     memset ( ticket, 0, sizeof (tag) );

     retval = recvmsg ( sock, &msg, 0 );
     if ( retval != sizeof (tag) )
     {
          UI_msgbox ( "ERROR",
                      "Ticket receipt failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     return ( 0 );
}


static int sim_load_overlay ( int sock, uint16_t unit )
{
     tag ticket;
     
     uint32_t words_recvd;
     uint32_t words_seq;


     words_recvd = 0;
     words_seq   = 0;

     while ( 1 )
     {
          memset ( &ticket, 0, sizeof ticket );
          ticket.s          = TAG_XFR;
          ticket.t.xfr.unit = unit;
          ticket.t.xfr.seq  = words_seq++;

          if ( sim_dispatch_ticket ( sock, &ticket ) < 0 )
          {
               UI_msgbox ( "ERROR",
                           "Ticket dispatch failed",
                           "Press any key to abort" );
               return ( -1 );
          }

          memcpy ( exe.cursor, bmap, (size_t)ticket.t.xfr.bsz * 2 );

          words_recvd += (uint32_t)ticket.t.xfr.bsz;

          snprintf ( opr, 120, "overlay load %i words -> 0x%08x",
                     ticket.t.xfr.bsz,
                     (uint32_t)(exe.cursor - map_exe));
          exe.cursor += ticket.t.xfr.bsz;

          sim_update_allocations ( );

          if ( ticket.t.xfr.bsz < BOARD_SIZE )
               /* That was the last one */
               break;
     }

     return ( words_recvd );
}

static int  sim_register_srf
( struct atria_srf * srf, struct atria_sisr * sisr, uint8_t intr )
{
     struct block_index * block_cursor;
     struct atria_sisr  * sisr_cursor;

     struct atria_srf * new_srf;

     uint16_t sid, i, trap;
     uint8_t subidx;

     sid = srf->soc.sid;

     if ( !map_hdr->srf_alpha )
     {
          
          /* we need to init the first block first */
          /* point cursor to next free word after EXE */
          map_hdr->srf_alpha = (map_hdr->img_ext + 1);
          map_hdr->img_ext +=
               (sizeof (struct block_index) / 2) +
               (sizeof (struct block_index) % 2);

          block_cursor = (struct block_index *)&map_exe[map_hdr->srf_alpha];

          memset ( block_cursor, 0, sizeof (struct block_index) );
          fsync ( mapfd );

          snprintf ( opr, 120,
                     "Registered SRF Alpha Index Block IMG:%08x",
                     map_hdr->srf_alpha );
          sim_update_allocations ( );

          /* Allocate the sisr table */
          /* First we need to make a small block which contains */
          /* the interrupt trap routine, then we will link */
          /* all SISRS back to that */
          trap = ++(map_hdr->img_ext);
          map_exe[map_hdr->img_ext]     = ISR_TRAP_1;
          map_exe[++(map_hdr->img_ext)] = ISR_TRAP_2;

          map_hdr->sisr_top = (map_hdr->img_ext + 1);
          sisr_cursor =
               (struct atria_sisr *)&map_exe[map_hdr->sisr_top];

          /* The problem with SISR is that it's not word-aligned */
          /* so we need to allocate the entire block as one */
          map_hdr->img_ext +=
               ((sizeof (struct atria_sisr) * (INTR_MAX + 1)) / 2) +
               ((sizeof (struct atria_sisr) * (INTR_MAX + 1)) % 2);

          for ( i = 0; i <= INTR_MAX; i++ )
          {
               memset ( &sisr_cursor[i], 0, sizeof (struct atria_sisr) );

               /* Link to the trap */
               sisr_cursor[i].isr = trap;
          }

          fsync ( mapfd );

          snprintf ( opr, 120,
                     "Registered SISR with %i slots at IMG:%08x",
                     (INTR_MAX + 1), map_hdr->sisr_top );
          sim_update_allocations ( );
          
     }

     /* set block index to alpha, then seek up */
     block_cursor = (struct block_index *)&map_exe[map_hdr->srf_alpha];

     /* We dont need to check for bad blocks, since that's stupid if */
     /* it did happen.. somehow */

     /* Trace srf, make blocks when we need to */
     for ( i = 0; i < 3; i++ )
     {
          /* Calculate the block index for this round */
          subidx =
               (uint8_t)( (sid & (0xf << (12 - (4 * i)))) >> (12 - (4 * i)));
          
          /* Check if we have an index for alpha */
          if ( !block_cursor->index[subidx] )
          {
               /* Need to make a new one */
               /* Note that the pointers are byte pointers */
               /* otherwise we'd have to multiply it "on-board" */
               block_cursor->index[subidx] =
                    (uint16_t)((map_hdr->img_ext + 1) * 2);

               map_hdr->img_ext +=
                    (sizeof (struct block_index) / 2) +
                    (sizeof (struct block_index) % 2);

               in_mod[0] = '\0';
               out_mod[0] = '\0';
               snprintf ( opr, 120,
                          "New SRF index block for %04X -> #%X at EXE:0x%08x",
                          sid, subidx, block_cursor->index[subidx] / 2 );
               sim_update_allocations ( );
               
               block_cursor =
                    (struct block_index *)
                    &map_exe[block_cursor->index[subidx] / 2];
               
               memset ( block_cursor, 0, sizeof (struct block_index) );

               
          }
          else
          {

               block_cursor =
                    (struct block_index *)
                    &map_exe[block_cursor->index[subidx] / 2];
          }

          
     }

     /* We are at gamma */
     subidx = (uint8_t)(sid & 0x000f);
     if ( block_cursor->index[subidx] )
     {
          /* already registered! */
          UI_msgbox ( "ERROR",
                      "SID already registered (1)",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* We are good! Lets allocate a new SRF */
     block_cursor->index[subidx] =
          (uint16_t)((map_hdr->img_ext + 1) * 2);

     map_hdr->img_ext +=
          (sizeof (struct atria_srf) / 2) +
          (sizeof (struct atria_srf) % 2);     

     /* Copy it in */
     memcpy ( (struct atria_srf *)&map_exe[block_cursor->index[subidx] / 2],
              srf, sizeof (struct atria_srf) );
     
     in_mod[0] = '\0';
     out_mod[0] = '\0';
     snprintf ( opr, 120,
                "Registered %04X%% with SRF at EXE:0x%08x",
                sid, block_cursor->index[subidx] );
     exe.cursor = &map_exe[block_cursor->index[subidx]];
     sim_update_allocations ( );

     /* Finally, if we have a SISR, we register that too*/
     if ( sisr )
     {
          /* We don't check for existing in this case */
          if ( intr > INTR_MAX )
          {
               UI_msgbox ( "ERROR",
                           "Interrupt number out of range",
                           "Press any key to abort" );
               return ( -1 );
          }

          sisr_cursor = (struct atria_sisr *)&map_exe[map_hdr->sisr_top];

          memcpy ( &sisr_cursor[intr], sisr, sizeof (struct atria_sisr) );

          in_mod[0] = '\0';
          out_mod[0] = '\0';
          snprintf ( opr, 120,
                     "Registered ISR#%i", intr );
          sim_update_allocations ( );
     }


}

static struct atria_srf * sim_pull_srf ( uint16_t sid )
{
     struct block_index * block;
     int i;
     uint8_t subidx;
     
     block = (struct block_index *)&map_exe[map_hdr->srf_alpha];

     for ( i = 0; i < 3; i++ )
     {
          subidx =
               (uint8_t)( (sid & (0xf << (12 - (4 * i)))) >> (12 - (4 * i)));

          if ( !block->index[subidx] )
               return ( NULL );

          block = (struct block_index *)&map_exe[block->index[subidx] / 2];
     }

     if ( !block->index[sid & 0x000f] )
          return ( NULL );
     
     return ( (struct atria_srf *)&map_exe[block->index[sid & 0x000f] / 2] );
}


static int  sim_load_master ( int sock )
{
     /* First step is to request the exec partition tag */
     tag ticket, query;
     tag_adr extnt;
     tag_uni ovrly;

     struct atria_srf srf;
     
     uint16_t i, align;

     memset ( &ticket,   0, sizeof ticket  );
     memset ( &query,    0, sizeof query   );

     memset ( &srf,  0, sizeof srf );

     ticket.s           = TAG_PRT;
     ticket.t.prt.ident = PART_EXE;

     /* Set-up out SID first */
     srf.soc.sid = reg_sid;

     if ( sim_dispatch_ticket ( sock, &ticket ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Ticket dispatch failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Check the reply */
     if ( (ticket.s != TAG_PRT) ||
          (ticket.t.prt.ident != PART_EXE) )
     {
          UI_msgbox ( "ERROR",
                      "Invalid ticket",
                      "Press any key to abort" );
          return ( -1 );
     }


     /* OK, so we have the EXE partition. First */
     /* we check the word size, it should be 16 */
     memset ( &query,   0, sizeof query  );

     query.s           = TAG_DAT;
     query.t.dat.ident = ticket.t.prt.word;
     
     if ( sim_dispatch_ticket ( sock, &query ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Ticket dispatch failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     if ( query.t.dat.data != 16 )
     {
          UI_msgbox ( "ERROR",
                      "word size incorrect",
                      "Press any key to abort" );
          return ( -1 );
     }


     /* We can ignore alignment since this is going in at 0 */
     /* so next we pull in the extent address tag */
     
     memset ( &query,   0, sizeof query  );

     query.s = TAG_ADR;
     query.t.adr.ident = ticket.t.prt.extnt;
     if ( sim_dispatch_ticket ( sock, &query ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Ticket dispatch failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     memcpy ( &extnt, &query.t.adr, sizeof extnt );
     
     /* Pull the data tag for offset */
     memset ( &query,   0, sizeof query  );
     query.s = TAG_DAT;
     query.t.dat.ident = extnt.offs;
     if ( sim_dispatch_ticket ( sock, &query ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Ticket dispatch failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     if ( query.t.dat.data > EXE_MAX )
     {
          UI_msgbox ( "ERROR",
                      "Exec partition too large",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Note that the ticket received gives the */
     /* offset of the extent for the partition, from zero */
     exe.ext = (uint32_t)query.t.dat.data;
     exe.cursor = &map_exe[exe.off];

     /* Ok, let's load in the executive overlay */
     /* We don't actually need to load in the overlay tag */
     /* we just need to reference it in a trasfer slip.   */
     /* But first we need to make sure its not zero.      */
     if ( !ticket.t.prt.ovrly )
     {
          UI_msgbox ( "ERROR",
                      "Invalid overlay",
                      "Press any key to abort" );
          return ( -1 );
     }


     if ( sim_load_overlay ( sock, ticket.t.prt.ovrly )
          != (exe.ext + 1) )
     {
          
          UI_msgbox ( "ERROR",
                      "Overlay failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Now we can start to set-up the srf */
     /* What is interesting to not is, the variables actually */
     /* being stored here (any any pointer that's not PC-related */
     /* is a byte address */
     *((uint16_t *)srf.text.top) = (uint16_t)(exe.off * 2);
     srf.text.top[2] = (uint8_t)((exe.off * 2) >> 16);
     *((uint16_t *)srf.text.bot) = (uint16_t)((exe.off + exe.ext) * 2);
     srf.text.bot[2] = (uint8_t)(((exe.off + exe.ext) * 2) >> 16);



     /* Next, we set up the Operational Partitions       */
     /* OP1 is funtionally equivilent to .data (for now) */
     /* However, the size may be zero                    */

     /* According to the specification, if a partition   */
     /* does not exist, we should receive an ident zero  */
     /* tag back when we request it. We do that first */
     memset ( &ticket,   0, sizeof query  );
     ticket.s = TAG_PRT;
     ticket.t.prt.ident = PART_OP1;
     if ( sim_dispatch_ticket ( sock, &ticket ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Ticket dispatch failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Look, lets be honest. This is not a critical program */
     /* it's just a prototyping tool. We're going to stop    */
     /* checking every ticket dispatch operation from now on */
     if ( ticket.t.prt.ident == PART_OP1 )
     {
          /* Check word size (should be 8) */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket.t.prt.word;
          sim_dispatch_ticket ( sock, &query );

          if ( query.t.dat.data != 8 )
          {
               UI_msgbox ( "ERROR",
                           "Invalid OP Parition word size",
                           "Press any key to abort" );
               return ( -1 );
          }

          /* load alignment */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket.t.prt.align;
          sim_dispatch_ticket ( sock, &query );

          align = (uint16_t)query.t.dat.data;

          /* Load extent */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_ADR;
          query.t.adr.ident = ticket.t.prt.extnt;
          sim_dispatch_ticket ( sock, &query );
          memcpy ( &extnt, &query.t.adr, sizeof extnt );

          /* Find actual extent */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = extnt.offs;
          sim_dispatch_ticket ( sock, &query );

          /* OP1 goes to the top of our spot in */
          /* memory. So we set the extent, and  */
          /* then set the OP2 offset to OP1's   */
          /* extent +1 */

          op1.ext = (uint32_t)query.t.dat.data;
          op2.off = (op1.off + op1.ext) + 1;

          /* Check that this actually works */
          if ( op2.off > OP_MAX )
          {
               UI_msgbox ( "ERROR",
                           "Operations Memory Insufficient",
                           "Press any key to abort" );
               return ( -1 );
          }

          snprintf ( opr, 120, "OP1 registered: %04x:%04x",
                     op1.off, (op1.off + op1.ext) );
          sim_update_allocations ( );

          /* Now we will prepare the exe partition to */
          /* receive the data as well, by updating the */
          /* extent to incorproate the data initialization */
          /* overlay, and setting the cursor to the start */
          /* of it */
          *((uint16_t *)srf.data.ofs) = (uint16_t)((exe.off + exe.ext + 1) * 2);
          srf.data.ofs[3] =
               (uint8_t)(((exe.off + exe.ext + 1) * 2) >> 16);
          exe.cursor = &map_exe[exe.off + exe.ext + 1];
          exe.ext += ((op1.ext / 2) + (op1.ext % 2));

          if ( exe.ext > EXE_MAX )
          {
               UI_msgbox ( "ERROR",
                           "Insufficient storage available",
                           "Press any key to abort" );
               return ( -1 );
          }
                                
          srf.data.lda = (uint16_t)op1.off;
          srf.data.siz = (uint16_t)(op1.ext + 1);

          /* Finally we can pull the overlay */
          if ( sim_load_overlay ( sock, ticket.t.prt.ovrly )
               != ((op1.ext / 2) + 1) )
          {
               UI_msgbox ( "ERROR",
                           "Overlay failed",
                           "Press any key to abort" );
               return ( -1 );
          }

          /* Update the global image */
          map_hdr->op_ext = (op1.off + op1.ext);

     }

     /* OP2 is relativly easier than OP1, we just need to set */
     /* the extent, and that's really it */
     memset ( &ticket,   0, sizeof query  );
     ticket.s = TAG_PRT;
     ticket.t.prt.ident = PART_OP2;
     sim_dispatch_ticket ( sock, &ticket );
     if ( ticket.t.prt.ident == PART_OP2 )
     {
          
          /* Check word size (should be 8) */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket.t.prt.word;
          sim_dispatch_ticket ( sock, &query );

          if ( query.t.dat.data != 8 )
          {
               UI_msgbox ( "ERROR",
                           "Invalid OP Parition word size",
                           "Press any key to abort" );
               return ( -1 );
          }

          /* load alignment */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket.t.prt.align;
          sim_dispatch_ticket ( sock, &query );

          align = (uint16_t)query.t.dat.data;

          /* Load extent */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_ADR;
          query.t.adr.ident = ticket.t.prt.extnt;
          sim_dispatch_ticket ( sock, &query );
          memcpy ( &extnt, &query.t.adr, sizeof extnt );

          /* Find actual extent */
          memset ( &query,   0, sizeof query  );
          query.s = TAG_DAT;
          query.t.dat.ident = extnt.offs;
          sim_dispatch_ticket ( sock, &query );

          op2.ext = (uint32_t)query.t.dat.data;

          /* Check that this actually works */
          if ( (op2.off + op2.ext) > OP_MAX )
          {
               UI_msgbox ( "ERROR",
                           "Operations Memory Insufficient",
                           "Press any key to abort" );
               return ( -1 );
          }
          
          /* Update the srf */
          srf.bss.lda = (uint16_t)op2.off;
          srf.bss.siz = (uint16_t)(op2.ext + 1);

          snprintf ( opr, 120, "OP2 registered: %04x:%04x",
                     op2.off, (op2.off + op2.ext) );
          sim_update_allocations ( );

          /* Update the global image */
          map_hdr->op_ext = (op2.off + op2.ext);

     }

     /* Update the executive total */
     map_hdr->img_ext = (exe.off + exe.ext);

     /* The time has now come to do the commissioning */

     /* ok now lets just iterate through all the commissions */
     /* for fun */
     for ( i = 1;; i++ )
     {
          memset ( &query,   0, sizeof query  );
          query.s = TAG_COM;
          query.t.com.ident = i;
          sim_dispatch_ticket ( sock, &query );

          if ( !query.t.com.ident )
               /* that's all folks */
               break;

          /* Send it off */
          if ( sim_exec_commission ( sock, &query, &srf ) < 0 )
          {
               UI_msgbox ( "ERROR",
                           "Commission Failed",
                           "Press any key to abort" );
               return ( -1 );
          }

          if (Auto.at)
               continue;
          
          wmove ( actscr, 2, 2 );
          wprintw ( actscr, "com: %i", i );
          wrefresh ( actscr );

          sim_update_allocations ( );

     }

     /* We're done with the package, tell them as much */
     memset ( &query, 0, sizeof query );
     query.s = 0xffff;
     sim_dispatch_ticket ( sock, &query );

     /* Finally, register the srf */
     if ( sim_register_srf ( &srf, NULL, 0 ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Registration Failed",
                      "Press any key to abort" );
          return ( -1 );
     }

     return ( 0 );


}


static int  sim_exec_commission
( int sock, tag * ticket, struct atria_srf * srf  )
{
     tag target, offset;
     
     /* We set up the cursor and then dispatch it */
     /* accordingly */
     memset ( &target,   0, sizeof target );
     target.s = TAG_ADR;
     target.t.adr.ident = ticket->t.com.endp;

     sim_dispatch_ticket ( sock, &target );

     if ( ( target.t.adr.part != PART_EXE ) &&
          ( ticket->t.com.endp > 0) &&
          ( ticket->t.com.type != GEN_TXA ) )
     {
          UI_msgbox ( "ERROR",
                      "Endpoint not in EXE",
                      "Press any key to abort" );
          return ( -1 );
     }


     memset ( &offset, 0, sizeof offset);
     offset.s = TAG_DAT;
     offset.t.dat.ident = target.t.adr.offs;
     sim_dispatch_ticket ( sock, &offset );

     exe.cursor = &map_exe[exe.off + offset.t.dat.data];

     switch ( ticket->t.com.type )
     {


          case GEN_DDR:
          case GEN_TXA:
          case GEN_ISR:
          case GEN_SEC:
          case GEN_PRI:
               /* Dispatch Desk Registration */
               if ( sim_exec_gen_spec ( sock, ticket, srf ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                                "Special Tag Failure",
                                "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

          case GEN_MOD:
               if ( sim_exec_gen_mod ( sock, ticket ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                      "GEN_REL Commission Failed",
                      "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

          case GEN_JMP:
               /* jmp/call */
               if ( sim_exec_gen_jmp ( sock, ticket ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                                "GEN_JMP Commission Faied",
                                "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

          case GEN_REL:
               /* Don't need to do this, for now */
               return ( 0 );



          case AVR_ILO:
          case AVR_IHI:
          case AVR_INL:
          case AVR_INH:
          case AVR_IWL:
          case AVR_IWH:
               if ( sim_exec_gen_imm ( sock, ticket ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                      "GEN_LDI Commission Failed",
                      "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

          case AVR_ML8:
          case AVR_MH8:
               if ( sim_exec_gen_mod ( sock, ticket ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                      "AVR_GEN (8) Commission Failed",
                      "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

          case XERIS_FLATLINE:
          case XERIS_ATRIA:
          case XERIS_LINK_SRF:
          case XERIS_LINK_SISR:
               if ( reg_sid > 2 )
               {
                    UI_msgbox ( "ERROR",
                                "core tags for non-core registrants",
                                "Press any key to abort" );
                    return ( -1 );
               }

               if ( sim_exec_gen_spec ( sock, ticket, srf ) < 0 )
               {
                    UI_msgbox ( "ERROR",
                                "Special Tag Failure",
                                "Press any key to abort" );
                    return ( -1 );
               }
               return ( 0 );

               
          default:
               break;

     }
     
     UI_msgbox ( "ERROR",
                 "Unsupported commission type",
                 "Press any key to abort" );
     return ( -1 );
               

}

static int  sim_exec_gen_rel ( int sock, tag * ticket )
{
     /* So it turns out, for the time being, we actually */
     /* never need to mess with PC-relatives. Since the  */
     /* Secretariat was compiled as a package, all the   */
     /* existing PC-rels are guarunteed to be correct as is */
     /* since they wont be calling outside... */

     /* So this is a stub for now */

     
     uint16_t endp, ins, target, tid;
     int16_t solution;
     tag query;

     
     /* So we have a RC-Relative commission tag. */
     /* This covers all avr PC-relative operations which */
     /* are as follows: */
     /* Opcode  bit pattern */
     /* (13-bit) */
     /* rjmp    1100 kkkk kkkk kkkk */
     /* rcall   1101 kkkk kkkk kkkk */
     /* (7-bit) */
     /* brbc    1111 01kk kkkk ksss */
     /* brbs    1111 00kk kkkk ksss */

     /* First lets load the actual instruction word */
     /* Cursor is already pointing at it */
     ins = *exe.cursor;

     /* Next, lets see if we have a target unit or addend */
     /* And then we can determine the target address      */
     if ( ticket->t.com.annx > 0 )
     {
          /* We should never have an annex and a reference */
          if ( ticket->t.com.ref > 0 )
          {
               UI_msgbox ( "ERROR",
                           "GEN_REL: annex with ref",
                           "Press any key to abort" );
               return ( -1 );
          }
          
          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket->t.com.annx;
          sim_dispatch_ticket ( sock, &query );

          /* Now note that for GEN_REL, if we have */
          /* an addend, it will be a byte address */
          /* so we need to convert it to a word  */
          /* address */

          target = (uint16_t)query.t.dat.data / 2;
     }
     else
     {
          /* Must be a unit reference. */

          /* Load the unit's "top" endpoint */
          memset ( &query, 0, sizeof query );
          query.s = TAG_UNI;
          query.t.uni.ident = ticket->t.com.ref;
          sim_dispatch_ticket ( sock, &query );
          tid = query.t.uni.top;

          memset ( &query, 0, sizeof query );
          query.s = TAG_ADR;
          query.t.adr.ident = tid;
          sim_dispatch_ticket ( sock, &query );
          if ( query.t.adr.part != PART_EXE )
          {
               UI_msgbox ( "ERROR",
                           "GEN_REL: ref not in EXE (2)",
                           "Press any key to abort" );
               return ( -1 );
          }    
          tid = query.t.adr.offs;

          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = tid;
          sim_dispatch_ticket ( sock, &query );

          /* This one is a word address */
          target = (uint16_t)query.t.dat.data;
     }


     
}



static int  sim_exec_gen_mod ( int sock, tag * ticket )
{
     /* The caviat about these is that we need to be   */
     /* aware of wheather the reference (if any) */
     /* is in EXE or OP1/2, because if it is in EXE */
     /* then we need to convert the address to a  */
     /* byte address. Other than that, its just a matter */
     /* of slapping it in, plus the addend */

     /* Target it already in cursor */
     uint32_t target;
     uint16_t part, new_val, tid, addend;
     tag query;

     /* Get target "pure" address */
     memset ( &query, 0, sizeof query );
     query.s = TAG_ADR;
     query.t.adr.ident = ticket->t.com.endp;
     sim_dispatch_ticket ( sock, &query );
     tid = query.t.adr.offs;

     memset ( &query, 0, sizeof query );
     query.s = TAG_DAT;
     query.t.dat.ident = tid;
     sim_dispatch_ticket ( sock, &query );
     target = (uint32_t)query.t.dat.data;
     
     /* get the reference (if any) */
     if ( ticket->t.com.ref > 0 )
     {
          memset ( &query, 0, sizeof query );
          query.s = TAG_UNI;
          query.t.uni.ident = ticket->t.com.ref;
          sim_dispatch_ticket ( sock, &query );
          tid = query.t.uni.top;

          memset ( &query, 0, sizeof query );
          query.s = TAG_ADR;
          query.t.adr.ident = tid;
          sim_dispatch_ticket ( sock, &query );
          part = query.t.adr.part;
          tid = query.t.adr.offs;
               
          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = tid;
          sim_dispatch_ticket ( sock, &query );

          if ( (part == PART_OP1) || (part == PART_OP2) )
          {
               new_val = (uint16_t)query.t.dat.data;
               new_val += op1.off;
          }
          else if ( part == PART_EXE )
          {
               new_val = (uint16_t)((exe.off + query.t.dat.data) * 2);
          }
          else
          {
               UI_msgbox ( "ERROR",
                           "GEN_MOD: ref parition not supported",
                           "Press any key to abort" );
               return ( -1 );
          }
     }
          

     /* Get addend value (if any) */
     if ( ticket->t.com.annx > 0 )
     {
          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket->t.com.annx;
          sim_dispatch_ticket ( sock, &query );
          /* addend */
          addend = (uint16_t)query.t.dat.data;
          new_val += addend;
     }
     else
          addend = 0;


     /* Note that, for the time-being, the source can't properly */
     /* distinguish OP1 and OP2, because ELF files don't do that */
     /* So we need to use the OP1.off offset for both. But otherwise */
     /* it could be selected here */
     if ( (part == PART_OP1) || (part == PART_OP2) )
          snprintf ( opr,    120, "GEN_MOD EXE:%08x -> OP1:%04x + 0x%04x",
                     target, new_val - op1.off, addend );
     else
          snprintf ( opr,    120, "GEN_MOD EXE:%08x -> EXE:%04x + 0x%04x",
                     target, (new_val / 2) - exe.off, addend );

     /* Take a snapshot */
     snprintf ( in_mod, 120, "0x%04x", *exe.cursor );

     /* Now, we need to bware of special AVR_ML8 and AVR_MH8 */
     /* in which case we apply only the high or low 8 bits */
     /* But AVR_ML8s are a bit weird.. The target will be */
     /* an odd byte, potentially, which will end up getting */
     /* rounded "down".  */
     if ( ticket->t.com.type == AVR_ML8 )
     {
          *exe.cursor &= 0xff00;
          *exe.cursor += (new_val & 0x00ff);
     }
     else if ( ticket->t.com.type == AVR_MH8 )
     {
          *exe.cursor &= 0x00ff;
          *exe.cursor += (new_val & 0xff00);
     }
     else
     {
          *exe.cursor = (uint16_t)new_val;
     }

     snprintf ( out_mod, 120, "0x%04x", *exe.cursor );

     return ( 0 );

}


static int  sim_exec_gen_imm ( int sock, tag * ticket )
{

     /* GCC does this thing with immediates that come */
     /* from 16-bit addresses. We'd like to use LDI  */
     /* GEN_LDI, but that requires a reliable pattern */
     /* which we wont always have. Therefore we've    */
     /* provided a catch-all set of special commissions */
     /* for AVR */

     /* This part handles all commissions that involve */
     /* immediates. This includes the following commands */

     /* andi  0111 KKKK dddd KKKK */
     /* cpi   0011 KKKK dddd KKKK */
     /* ldi   1110 KKKK dddd KKKK */
     /* ori   0110 KKKK dddd KKKK */
     /* sbci  0100 KKKK dddd KKKK */
     /* subi  0101 KKKK dddd KKKK */

     /* There are some that don't fit the patten, and */
     /* also don't seem to used in a way which would */
     /* need immediate values loaded from an address */
     /* The rub likely lies in the fact that these   */
     /* can't take 8-bit immediates. And it's probably */
     /* the case that such immediate adding is always */
     /* sone with subi+sbci with negative values, hence */
     /* AVR_LO8_LDI_NEG */
     /* for now, we will exclude these ones */
     /* adiw  1001 0110 KKdd KKKK */
     /* sbiw  1001 0111 KKdd KKKK */

     /* So as we can see, all the others show  */
     /* a clear pattern. We just will check */
     /* to make sure everything is as expected, */
     /* and will otherwise use them for the display */


     uint8_t dreg, hi, word;
     uint32_t target;
     uint16_t part, objective, orig, tid;
     tag query;

     char opc_string[5];
     char com_code[8];
     char part_code[4];
     char neg;

     /* verify tag type */
     switch ( ticket->t.com.type )
     {
          case AVR_ILO:
               hi = 0;
               word = 0;
               neg = ' ';
               strcpy ( com_code, "AVR_ILO" );
               break;

          case AVR_IHI:
               hi = 1;
               word = 0;
               neg = ' ';
               strcpy ( com_code, "AVR_IHI" );
               break;

          case AVR_INL:
               hi = 0;
               word = 0;
               neg = '-';
               strcpy ( com_code, "AVR_INL" );
               break;

          case AVR_INH:
               hi = 1;
               word = 0;
               neg = '-';
               strcpy ( com_code, "AVR_INH" );
               break;

          case AVR_IWL:
               hi = 0;
               word = 1;
               neg = ' ';
               strcpy ( com_code, "AVR_IWL" );
               break;

          case AVR_IWH:
               hi = 1;
               word = 1;
               neg = ' ';
               strcpy ( com_code, "AVR_IWL" );
               break;
               

          default:
               UI_msgbox ( "ERROR",
                           "GEN_LDI: Tag type invalid",
                           "Press any key to abort" );
               return ( -1 );
     }


     /* verify ldi opcode (top 4 bits) */
     switch ( (*exe.cursor) >> 12 )
     {
          case AVR_OPC_ANDI:
               strcpy ( opc_string, "andi" );
               break;

          case AVR_OPC_CPI:
               strcpy ( opc_string, "cpi" );
               break;

          case AVR_OPC_LDI:
               strcpy ( opc_string, "ldi" );
               break;

          case AVR_OPC_ORI:
               strcpy ( opc_string, "ori" );
               break;

          case AVR_OPC_SBCI:
               strcpy ( opc_string, "sbci" );
               break;

          case AVR_OPC_SUBI:
               strcpy ( opc_string, "subi" );
               break;

          default:
               UI_msgbox ( "ERROR",
                           "GEN_LDI: Target opcode unsupported",
                           "Press any key to abort" );
               return ( -1 );          
     }


     

     /* Get the destination register */
     dreg = (((*exe.cursor) >> 4) & 0x0f) + 16;

     /* get the original value */
     orig = ((*exe.cursor) & 0x0f00) >> 4;
     orig += (*exe.cursor) & 0x000f;
     
     /* Get target "pure" address */
     memset ( &query, 0, sizeof query );
     query.s = TAG_ADR;
     query.t.adr.ident = ticket->t.com.endp;
     sim_dispatch_ticket ( sock, &query );
     tid = query.t.adr.offs;

     memset ( &query, 0, sizeof query );
     query.s = TAG_DAT;
     query.t.dat.ident = tid;
     sim_dispatch_ticket ( sock, &query );
     target = (uint32_t)query.t.dat.data;

     /* get the reference (if any) */
     if ( ticket->t.com.ref > 0 )
     {
          memset ( &query, 0, sizeof query );
          query.s = TAG_UNI;
          query.t.uni.ident = ticket->t.com.ref;
          sim_dispatch_ticket ( sock, &query );
          tid = query.t.uni.top;

          memset ( &query, 0, sizeof query );
          query.s = TAG_ADR;
          query.t.adr.ident = tid;
          sim_dispatch_ticket ( sock, &query );
          part = query.t.adr.part;
          tid = query.t.adr.offs;


          /* Get reference address */
          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = tid;
          sim_dispatch_ticket ( sock, &query );

          /* Load reference address */
          objective = (uint16_t)query.t.dat.data;


     }

     /* Apply addend value (if any) */
     if ( ticket->t.com.annx > 0 )
     {
          memset ( &query, 0, sizeof query );
          query.s = TAG_DAT;
          query.t.dat.ident = ticket->t.com.annx;
          sim_dispatch_ticket ( sock, &query );
          objective += (uint16_t)query.t.dat.data;
     }

     /* Apply the parititon offset */
     switch ( part )
     {
          case PART_OP1:
               objective += op1.off;
               strcpy ( part_code, "OP1" );
               break;
               
          case PART_OP2:
               objective += op1.off;
               strcpy ( part_code, "OP2" );
               break;

          case PART_EXE:
               objective += exe.off;
               strcpy ( part_code, "EXE" );
               break;

          default:
               UI_msgbox ( "ERROR",
                           "GEN_LDI: Invalid partition ref",
                           "Press any key to abort" );
               return ( -1 );
     }

     /* Adjust the objective for word/byte address */
     /* as necessary (only really for "AVR_LO8_LDI_GS */
     if ( part == PART_EXE && !word )
     {
          /* This usually is a set up for either: */
          /* 1: lpm (AVR_LO8) or  */
          /* 2: ijmp/call/func ptr (AVR_LO8_GS)  */
          /* For the former only, we need a byte address */
          objective *= 2;
     }


     /* Describe the operation */
     if ( !hi )
          snprintf ( opr, 120, "Immediate %s:%c0x%02x[%02x]/r%i",
                     part_code, neg,
                     (objective >> 8), (objective & 0x00ff), dreg );
     else
          snprintf ( opr, 120, "Immediate %s:%c0x[%02x]%02x/r%i",
                     part_code, neg,
                     (objective >> 8), (objective & 0x00ff), dreg );
     snprintf ( in_mod, 120, "%s r%i, 0x%02x", opc_string, dreg, orig ); 

     /* Apply negation */
     if ( neg == '-' )
     {
          /* Direct 2's complement */
          objective = ~objective;
          objective++;
     }

     /* Now we select lo/hi */
     if ( !hi )
          objective &= 0x00ff;
     else
          objective = objective >> 8;


     /* Orig */
     snprintf ( out_mod, 120, "%s r%i, 0x%02x",
                opc_string, dreg, objective );
     dreg -= 16;

     /* Rebuild the instruction */
     *exe.cursor &= 0xf0f0;
     *exe.cursor += (objective << 4) & 0x0f00;
     *exe.cursor += objective & 0x000f;

     return ( 0 );

}

static int  sim_exec_gen_jmp ( int sock, tag * ticket )
{
     uint16_t tid, comp;
     uint32_t objective, orig;
     tag query;
     char jmp, opc[5];
     
     /* Basically, we deal with jmp or call, that's it */

     /* jmp  1001 010k kkkk 110k  + 16 bit */
     /* call 1001 010k kkkk 111k  + 16 bit */

     /* As we can see, its the same pattern */
     /* We are expecting a defined symbol, not */
     /* an addend */

     if ( ticket->t.com.annx > 0 )
     {
          UI_msgbox ( "ERROR",
                      "GEN_JMP: Annex unexpected",
                      "Press any key to abort" );
          return ( -1 );
     }

     if ( !ticket->t.com.ref )
     {
          UI_msgbox ( "ERROR",
                      "GEN_JMP: No ref",
                      "Press any key to abort" );
          return ( -1 );
     }

     /* Detect the opcode */
     switch ( ((*exe.cursor) & 0x000e) >> 1 )
     {
          case AVR_OPC_JMP:
               jmp = 1;
               strcpy ( opc, "jmp" );
               break;

          case AVR_OPC_CALL:
               jmp = 0;
               strcpy ( opc, "call" );
               break;

          default:
               UI_msgbox ( "ERROR",
                           "GEN_JMP: Invalid opcode",
                           "Press any key to abort" );
               return ( -1 );
     }

     /* load the original address */
     comp = (*exe.cursor) & 1;
     comp += ((*exe.cursor) & 0x01f0) >> 3;
     orig = ((uint32_t)comp) << 16;
     orig += *(exe.cursor + 1);



     /* Lets load in our target */
     memset ( &query, 0, sizeof query );
     query.s = TAG_UNI;
     query.t.uni.ident = ticket->t.com.ref;
     sim_dispatch_ticket ( sock, &query );
     tid = query.t.uni.top;
     
     memset ( &query, 0, sizeof query );
     query.s = TAG_ADR;
     query.t.adr.ident = tid;
     sim_dispatch_ticket ( sock, &query );
     tid = query.t.adr.offs;

     if ( query.t.adr.part != PART_EXE )
     {
          UI_msgbox ( "ERROR",
                      "GEN_JMP: Ref not in EXE",
                      "Press any key to abort" );
          return ( -1 );
     }

     memset ( &query, 0, sizeof query );
     query.s = TAG_DAT;
     query.t.dat.ident = tid;
     sim_dispatch_ticket ( sock, &query );

     objective = (uint32_t)query.t.dat.data;

     /* Update the operation */
     snprintf ( opr, 120, "GEN_JMP -> EXE:0x%06x", objective );
     snprintf ( in_mod, 120, "%s 0x%06x",
                opc, orig );

     /* add exec offset */
     objective += (uint32_t)exe.off;
     snprintf ( out_mod, 120, "%s 0x%06x",
                opc, objective );

     /* split off the upper 6 bits */
     comp = ((uint16_t)(objective >> 16) & 0x003e) << 3;
     comp += (uint16_t)(objective >> 16) & 0x0001;

     /* Mask out the lower 16 bits */
     objective &= 0x0000ffff;

     /* we just need to mask off the existing instruction and */
     /* slot it right in */

     /* Mask out instruction */
     *exe.cursor &= 0xfe0e;
     /* Drop in high bits */
     *exe.cursor += (uint16_t)comp;
     /* the low word goes in the high part of memory */
     /* (yea, its backwards.. but in direction of PC) */
     *(exe.cursor + 1) = (uint16_t)objective;

     return ( 0 );

}


static int  sim_exec_gen_spec
( int sock, tag * ticket, struct atria_srf * srf )
{
     tag query;
     uint16_t tid, isr, office, comp;
     uint32_t objective;

     in_mod[0] = '\0';
     out_mod[0] = '\0';

     switch ( ticket->t.com.type )
     {
          case GEN_DDR:
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );

               srf->soc.dd =
                    (uint16_t)(query.t.dat.data + exe.off);

               snprintf ( opr, 120, "Dispatch Desk Registered EXE:0x%08x",
                          srf->soc.dd );

               /* Also register with opsec if it is FLATLINE% or ATRIA% */
               if ( reg_sid == 0x0001 )
               {
                    /* FLATLINE */
                    objective = srf->soc.dd;

                    /* split off the upper 6 bits */
                    comp = ((uint16_t)(objective >> 16) & 0x003e) << 3;
                    comp += (uint16_t)(objective >> 16) & 0x0001;

                    /* Mask out the lower 16 bits */
                    objective &= 0x0000ffff;
                    
                    /* we just need to mask off the existing instruction and */
                    /* slot it right in */

                    /* Mask out instruction */
                    map_exe[map_hdr->sys_opsec_flatline] &= 0xfe0e;
                    /* Drop in high bits */
                    map_exe[map_hdr->sys_opsec_flatline] += (uint16_t)comp;
                    /* the low word goes in the high part of memory */
                    /* (yea, its backwards.. but in direction of PC) */
                    map_exe[map_hdr->sys_opsec_flatline + 1]
                         = (uint16_t)objective;

                    snprintf ( opr, 120, "FLATLINE% Linked to EXE:0x%08x",
                          srf->soc.dd );
               }
               else if ( reg_sid == 0x0002 )
               {
                    /* ATRIA */
                    objective = srf->soc.dd;

                    /* split off the upper 6 bits */
                    comp = ((uint16_t)(objective >> 16) & 0x003e) << 3;
                    comp += (uint16_t)(objective >> 16) & 0x0001;

                    /* Mask out the lower 16 bits */
                    objective &= 0x0000ffff;
                    
                    /* we just need to mask off the existing instruction and */
                    /* slot it right in */

                    /* Mask out instruction */
                    map_exe[map_hdr->sys_opsec_atria] &= 0xfe0e;
                    /* Drop in high bits */
                    map_exe[map_hdr->sys_opsec_atria] += (uint16_t)comp;
                    /* the low word goes in the high part of memory */
                    /* (yea, its backwards.. but in direction of PC) */
                    map_exe[map_hdr->sys_opsec_atria + 1]
                         = (uint16_t)objective;

                    snprintf ( opr, 120, "ATRIA% Linked to EXE:0x%08x",
                          srf->soc.dd );
               }
                    
               

               return ( 0 );

          case GEN_TXA:
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );

               srf->soc.account =
                    (uint16_t)(query.t.dat.data + op1.off);

               snprintf ( opr, 120, "Transaction Account Registered OP1:0x%08x",
                          srf->soc.account );

               return ( 0 );

          case GEN_ISR:
               /* This can't be called from opsec */
               if ( !reg_sid )
               {
                    UI_msgbox ( "ERROR",
                                "GEN_SPEC: OPSEC cannot have ISRs",
                                "Press any key to abort" );
                    return ( -1 );
               }

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = ticket->t.com.annx;
               sim_dispatch_ticket ( sock, &query );
               isr = ((uint16_t *)&query.t.dat.data)[0];
               office = ((uint16_t *)&query.t.dat.data)[1];

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );
               
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );
               ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                    [isr].flags = 0x7f;

               ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                    [isr].flags = 0x7f;

               ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                    [isr].isr = (uint16_t)(query.t.dat.data + exe.off);

               ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                    [isr].office = office;

               
               snprintf ( opr, 120,
                          "ISR %i Registered -> EXE:0x%08x Office: %04X$",
                          ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                          [isr].isr,
                          ((struct atria_sisr *)&map_exe[map_hdr->sisr_top])
                          [isr].office );
               
               return ( 0 );

          case GEN_SEC:
               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = ticket->t.com.annx;
               sim_dispatch_ticket ( sock, &query );
               srf->soc.dsec = ((uint16_t *)&query.t.dat.data)[0];
               srf->soc.usec = ((uint16_t *)&query.t.dat.data)[1];

               snprintf ( opr, 120,
                          "Initial Security: Down: 0x%04x, Up: 0x%04x",
                          srf->soc.dsec, srf->soc.usec );

               return ( 0 );

          case GEN_PRI:
               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = ticket->t.com.annx;
               sim_dispatch_ticket ( sock, &query );
               srf->soc.bpri = (uint16_t)query.t.dat.data;

               snprintf ( opr, 120,
                          "Base priority: 0x%04x", srf->soc.bpri );

               return ( 0 );

          case XERIS_FLATLINE:
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );

               map_hdr->sys_opsec_flatline =
                    (uint32_t)(query.t.dat.data + exe.off);

               snprintf ( opr, 120, "FLATLINE%% Jump point EXE:0x%08x",
                          map_hdr->sys_opsec_flatline );

               return ( 0 );

          case XERIS_ATRIA:
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );

               map_hdr->sys_opsec_atria =
                    (uint32_t)(query.t.dat.data + exe.off);

               snprintf ( opr, 120, "ATRIA%% Jump point EXE:0x%08x",
                          map_hdr->sys_opsec_flatline );

               return ( 0 );

          case XERIS_LINK_SRF:
               /* SRF is loaded AVR_LDI8s for relevent ldi    */
               /* instructions that load the address of  */
               /* the relevent SRF alpha block */
               
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );

               /* Ok at the endpoint address, we have */
               /* two LDI instructions that need to  */
               /* receive the lo, then hi of the actual */
               /* (byte) address of the SRF alpha block, */
               map_exe[exe.off + query.t.dat.data]
                    &= 0xf0f0;
               map_exe[exe.off + query.t.dat.data]
                    += ((uint16_t)((map_hdr->srf_alpha * 2))) & 0x000f;
               map_exe[exe.off + query.t.dat.data]
                    += (((uint16_t)((map_hdr->srf_alpha * 2))) & 0x00f0) << 4;

               map_exe[exe.off + query.t.dat.data + 1]
                    &= 0xf0f0;
               map_exe[exe.off + query.t.dat.data + 1]
                    += ((uint16_t)((map_hdr->srf_alpha * 2) >> 8)) & 0x000f;
               map_exe[exe.off + query.t.dat.data + 1]
                    += (((uint16_t)((map_hdr->srf_alpha * 2) >> 8)) &
                        0x00f0) << 4;
               
               snprintf ( opr, 120, "SRF link at EXE:0x%08x",
                          exe.off + query.t.dat.data );

               
               return ( 0 );


          case XERIS_LINK_SISR:
               /* Similar to the SRF, we need to */
               /* "manually" link the symbol for */
               /* the SRF to the byte address of */
               /* the SRF table                  */
               memset ( &query, 0, sizeof query );
               query.s = TAG_ADR;
               query.t.adr.ident = ticket->t.com.endp;
               sim_dispatch_ticket ( sock, &query );
               tid = query.t.adr.offs;

               memset ( &query, 0, sizeof query );
               query.s = TAG_DAT;
               query.t.dat.ident = tid;
               sim_dispatch_ticket ( sock, &query );
               
               /* Ok at the endpoint address, we have */
               /* two LDI instructions that need to  */
               /* receive the lo, then hi of the actual */
               /* (byte) address of the SISR root, */
               map_exe[exe.off + query.t.dat.data]
                    &= 0xf0f0;
               map_exe[exe.off + query.t.dat.data]
                    += ((uint16_t)((map_hdr->sisr_top * 2))) & 0x000f;
               map_exe[exe.off + query.t.dat.data]
                    += (((uint16_t)((map_hdr->sisr_top * 2))) & 0x00f0) << 4;

               map_exe[exe.off + query.t.dat.data + 1]
                    &= 0xf0f0;
               map_exe[exe.off + query.t.dat.data + 1]
                    += ((uint16_t)((map_hdr->sisr_top * 2) >> 8)) & 0x000f;
               map_exe[exe.off + query.t.dat.data + 1]
                    += (((uint16_t)((map_hdr->sisr_top * 2) >> 8)) &
                        0x00f0) << 4;
               
               snprintf ( opr, 120, "SISR link at EXE:0x%08x",
                          exe.off + query.t.dat.data );

               
               return ( 0 );

          default:
               break;
     }
               
                          
     UI_msgbox ( "ERROR",
                 "GEN_SPEC: Bad commission tag",
                 "Press any key to abort" );
     return ( -1 );
}               


static void sim_dump_elf ( void )
{
     int elffd, i;
     WINDOW * dbox;

     Elf32_Ehdr * elf_header;
     Elf32_Shdr * sec_headers;
     Elf32_Phdr * prog_headers;
     
     char       * strtab;
     
     uint8_t * map_byte;
     uint16_t * text_word;

     uint32_t ehdr_offs, phdr_offs, shdr_offs, text_offs, strtab_offs;

     char map_file[120];
     off_t map_size;


     map_file[0] = '\0';

     if ( !Auto.pkg )
     {
          dbox = UI_cwin ( 5, 64,
                           "Enter Output ELF File Name",
                           "Press ENTER to Accept" );
          wattrset ( dbox, A_REVERSE | A_UNDERLINE );
          for ( i = 0; i < 60; i++ )
               mvwaddch ( dbox, 2, (i+2), ' ' );
          UI_prompt ( dbox, 2, 2, map_file, 60 );
          delwin ( dbox );
     }
     else
     {
          strncpy ( map_file, Auto.pkg, 120 );
          map_file[119] = '\0';
     }
         

     if ( !map_file[0] )
          return;

     elffd = open ( map_file, O_CREAT | O_RDWR | O_EXCL );

     if ( elffd < 0 )
     {
          strerror_r ( errno, map_file, 120 );
          UI_msgbox ( "Unable to Open",
                      map_file,
                      "Press any key to return" );
          return;
     }

     fchmod ( elffd, 00775 );

     map_size = (sizeof (uint16_t) * (map_hdr->img_ext + 1))
          + sizeof (Elf32_Ehdr) + (sizeof (Elf32_Phdr) * 2)
          + (sizeof (Elf32_Shdr) * 5) + (sizeof (char) * 120);
     
     
     i = ftruncate ( elffd, map_size );
     if ( i < 0 )
     {
          strerror_r ( errno, map_file, 120 );
          UI_msgbox ( "Unable to Truncate",
                      map_file,
                      "Press any key to return" );
          close ( mapfd );
          mapfd = -1;
          return;
     }

     map_byte = (uint8_t *) mmap ( 0, (size_t)map_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_NOCORE,
                                   elffd, 0 );

     if ( map_byte == MAP_FAILED )
     {
          UI_msgbox ( "ERROR",
                      "Unable to create memory map",
                      "Press any key to return" );
          return;
     }

     /* OK! lets get to it */

     ehdr_offs   = 0;
     phdr_offs   = sizeof (Elf32_Ehdr);
     shdr_offs   = phdr_offs + (sizeof (Elf32_Phdr) * 2);
     strtab_offs = shdr_offs + (sizeof (Elf32_Shdr) * 5);
     text_offs   = strtab_offs + (sizeof (char) * 120);
     
     elf_header = (Elf32_Ehdr *)&map_byte[ehdr_offs];
     prog_headers = (Elf32_Phdr *)&map_byte[phdr_offs];
     sec_headers = (Elf32_Shdr *)&map_byte[shdr_offs];
     strtab      = (char *)&map_byte[strtab_offs];
     text_word = (uint16_t *)&map_byte[text_offs];

     memset ( map_byte, 0, map_size );

     /* ELF header */
     elf_header->e_ident[EI_MAG0]    = ELFMAG0;
     elf_header->e_ident[EI_MAG1]    = ELFMAG1;
     elf_header->e_ident[EI_MAG2]    = ELFMAG2;
     elf_header->e_ident[EI_MAG3]    = ELFMAG3;
     elf_header->e_ident[EI_CLASS]   = ELFCLASS32;
     elf_header->e_ident[EI_DATA]    = ELFDATA2LSB;
     elf_header->e_ident[EI_VERSION] = EV_CURRENT;

     elf_header->e_type              = ET_EXEC;
     elf_header->e_machine           = EM_AVR;
     elf_header->e_version           = EV_CURRENT;
     elf_header->e_ehsize            = sizeof (Elf32_Ehdr);
     elf_header->e_flags             = E_AVR_MACH_AVR5;
     
     elf_header->e_phoff             = phdr_offs;
     elf_header->e_phentsize         = sizeof (Elf32_Phdr);
     elf_header->e_phnum             = 2;

     elf_header->e_shoff             = shdr_offs;
     elf_header->e_shentsize         = sizeof (Elf32_Shdr);
     elf_header->e_shnum             = 5;
     elf_header->e_shstrndx          = 4;

     /* Program headers */
     /* ".text" */
     prog_headers[0].p_type           = PT_LOAD;
     prog_headers[0].p_offset        = text_offs;
     prog_headers[0].p_filesz        = (map_hdr->img_ext + 1) * 2;
     prog_headers[0].p_memsz         = (map_hdr->img_ext + 1) * 2;
     prog_headers[0].p_flags         = PF_R | PF_X;
     prog_headers[0].p_align         = 2;

     /* ".bss" */
     prog_headers[1].p_type           = PT_LOAD;
     prog_headers[1].p_offset        = text_offs + ((map_hdr->img_ext + 1) * 2);
     prog_headers[1].p_vaddr         = 0x800000 + OP_MIN;
     prog_headers[1].p_paddr         = 0x800000 + OP_MIN;
     prog_headers[1].p_memsz         = (OP_MAX + 1);
     prog_headers[1].p_flags         = PF_R | PF_W;
     prog_headers[1].p_align         = 1;

     /* Section headers */
     /* .text */
     strcpy ( strtab, " .text" );
     sec_headers[1].sh_name          = 1;
     sec_headers[1].sh_type          = SHT_PROGBITS;
     sec_headers[1].sh_flags         = SHF_ALLOC | SHF_EXECINSTR;
     sec_headers[1].sh_addr          = 0;
     sec_headers[1].sh_offset        = text_offs;
     sec_headers[1].sh_size          = (map_hdr->img_ext + 1) * 2;
     sec_headers[1].sh_link          = 0;
     sec_headers[1].sh_info          = 0;
     sec_headers[1].sh_addralign     = 2;
     sec_headers[1].sh_entsize       = 0;

     /* .data */
     strcpy ( &strtab[7], ".data" );
     sec_headers[2].sh_name          = 7;
     sec_headers[2].sh_type          = SHT_PROGBITS;
     sec_headers[2].sh_flags         = SHF_WRITE | SHF_ALLOC;
     sec_headers[2].sh_addr          = 0x800000 + OP_MIN;
     sec_headers[2].sh_offset        = text_offs + ((map_hdr->img_ext + 1) * 2);
     sec_headers[2].sh_size          = 0;
     sec_headers[2].sh_link          = 0;
     sec_headers[2].sh_info          = 0;
     sec_headers[2].sh_addralign     = 1;
     sec_headers[2].sh_entsize       = 0;

     /* .bss */
     strcpy ( &strtab[13], ".bss" );
     sec_headers[3].sh_name          = 13;
     sec_headers[3].sh_type          = SHT_NOBITS;
     sec_headers[3].sh_flags         = SHF_WRITE | SHF_ALLOC;
     sec_headers[3].sh_addr          = 0x800000 + OP_MIN;
     sec_headers[3].sh_offset        = text_offs + ((map_hdr->img_ext + 1) * 2);
     sec_headers[3].sh_size          = OP_MAX + 1;
     sec_headers[3].sh_link          = 0;
     sec_headers[3].sh_info          = 0;
     sec_headers[3].sh_addralign     = 1;
     sec_headers[3].sh_entsize       = 0;

     /* .shstrtab */
     strcpy ( &strtab[18], ".shstrtab" );
     sec_headers[4].sh_name          = 18;
     sec_headers[4].sh_type          = SHT_STRTAB;
     sec_headers[4].sh_flags         = 0;
     sec_headers[4].sh_addr          = 0;
     sec_headers[4].sh_offset        = strtab_offs;
     sec_headers[4].sh_size          = 120;
     sec_headers[4].sh_link          = 0;
     sec_headers[4].sh_info          = 0;
     sec_headers[4].sh_addralign     = 1;
     sec_headers[4].sh_entsize       = 0;
     

     /* Copy the EXE partition */
     memcpy ( text_word, map_exe, sizeof (uint16_t) * (map_hdr->img_ext + 1));

     fsync ( elffd );
     munmap ( map_byte, map_size );
     close ( elffd );

     UI_msgbox ( "Complete",
                 "ELF written successfully",
                 "Press any key to return" );
     return;
     
}
