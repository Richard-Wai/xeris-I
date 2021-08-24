/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>

#include "globl.h"
#include "elf.h"
#include "ui.h"
#include "pack.h"
#include "atria.h"

#define PRT_POOL 8
#define ADR_POOL 10240
#define UNI_POOL 10240
#define COM_POOL 10240
#define DAT_POOL 10240
#define MAN_POOL 10240
#define MAN_SIZE 10240


int        bmapfd;
uint16_t * bmap;
tag tic_in, tic_out;

tag_xfr    act_xfr;
uint32_t   act_xfr_total, act_xfr_sent, act_xfr_mark;

WINDOW * tic_win_in, * tic_win_out;

struct elf_sections * sec_list;
struct elf_symbols  * sym_list;
Elf32_Rela          * rel_list;

int            sec_count, sym_count, rel_count;
uint16_t       text_unit, data_unit;

/* Registration Package */
struct
{
     tag_prt prt[PRT_POOL];
     tag_adr adr[ADR_POOL];
     tag_uni uni[UNI_POOL];
     tag_com com[COM_POOL];
     tag_dat dat[DAT_POOL];
     tag_man man[MAN_POOL];
} reg_pack;

int adr_free, uni_free, com_free, dat_free, man_free;

/* Human Reference Partition Data */
char man_data[MAN_SIZE];
uint32_t man_cursor;



static void update_ticket ( WINDOW *tw, tag * tic, const char * title );
static void update_tickets ( void );

static int  init_package ( uint16_t sid );

static void op_new_reg ( int sock, uint16_t sid );
static uint16_t op_make_man ( const char * name );
static uint16_t op_make_xtag ( uint16_t type, uint32_t addr, uint64_t annx );

static void op_tag_prt ( int sock );
static void op_tag_adr ( int sock );
static void op_tag_uni ( int sock );
static void op_tag_com ( int sock );
static void op_tag_dat ( int sock );
static void op_tag_man ( int sock );
static void op_tag_xfr ( int sock );

static uint16_t op_rela_com ( Elf32_Rela *rela );

extern void pack_base ( void )
{
     int sock, sock_pair[2], map, i;
     struct sockaddr_un addr;
     struct iovec msgvec;
     struct msghdr msg;
     struct cmsghdr * ctl;

     uint16_t sid;
     uint32_t spec_addr, spec_annx;
     Elf32_Sym * spec_sym;

     union
     {
          char           buf[ CMSG_SPACE(sizeof (int)) ];
          struct cmsghdr align;
     } mapxfer;

     char sock_path[120];
     

     
     int smax, smay;
     WINDOW * dbox;


     memset ( &bmapfd,   0, sizeof bmapfd   );
     memset ( &bmap,     0, sizeof bmap     );
     
     memset ( &msgvec,   0, sizeof msgvec   );
     memset ( &msg,      0, sizeof msg      );
     memset ( &mapxfer,  0, sizeof mapxfer  );

     memset ( &tic_in,   0, sizeof tic_in   );
     memset ( &tic_out,  0, sizeof tic_out  );

     memset ( sock_path, 0, 120             );

     memset ( &act_xfr,  0, sizeof act_xfr  );
     memset ( &reg_pack, 0, sizeof reg_pack );
     memset ( man_data,  0, MAN_SIZE        );
     memset ( &man_cursor, 0, sizeof man_cursor );

     sec_list = NULL;
     sym_list = NULL;
     rel_list = NULL;

     text_unit = 0;
     data_unit = 0;

     adr_free = 1;
     uni_free = 1;
     com_free = 1;
     dat_free = 1;
     man_free = 1;


     if (Auto.at)
          goto No_UI;
     
     /* This doesn't "stay running" in the background */
     /* every time is a new set-up */
     wclear ( regscr );
     UI_fwin ( regscr,
               "Package Host Tool",
               "" );
     touchwin ( regscr );

     actscr = regscr;
     wrefresh ( actscr );



     /* We try to set up as much stuff as possible before */
     /* we call it quits */
     getmaxyx ( actscr, smay, smax );
     smay--; smax--;

     tic_win_in = derwin
          (
               actscr,
               ( smay - 6 ),
               ( (smax / 2) - 6 ),
               3,
               3
          );

     tic_win_out = derwin
          (
               actscr,
               ( smay - 6 ),
               ( (smax / 2) - 6 ),
               3,
               ( (smax / 2) + 5 )
          );

     if ( (!tic_win_in) || (!tic_win_out) )
          err_nomem ( );


No_UI:
     /* Ok now we need to ask the user to provide the name */
     /* pipe to connect to                                 */

     sock = socket ( PF_UNIX, SOCK_DGRAM, 0 );
     if ( sock < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Cannot open UNIX Socket",
                      "Press any key to return" );
          goto quit;
     }


     while ( 1 )
     {
          memset ( &addr, 0, sizeof addr );

          if (!Auto.cpoint)
          {
               dbox = UI_cwin ( 5, 64,
                                "Enter Pipe to connect to",
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
          }

               if ( !sock_path[0] )
          {
               close ( sock );
               goto quit;
          }
          
          /* Attempt to connect */
          addr.sun_family = AF_UNIX;
          strncpy ( addr.sun_path, sock_path, 103 );
          addr.sun_len = SUN_LEN(&addr);

          i = connect ( sock, (struct sockaddr *)&addr, sizeof addr );
          if ( i == 0 )
               break;
          
          UI_msgbox ( "ERROR",
                      "Cannot connect to specified socket.",
                      "Press any key to retry" );

          if (Auto.at)
               goto quit;
     }

     touchwin ( actscr );
     wrefresh ( actscr );

     /* OK, send over the socket pair and then switch to it! */
     if ( socketpair ( PF_UNIX, SOCK_DGRAM, 0, sock_pair ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Could not create socket pair",
                      "Press any key to abort" );
          close ( sock );
          goto quit;
     }

     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;
     msg.msg_control          = &mapxfer;
     msg.msg_controllen       = sizeof mapxfer;

     ctl                      = CMSG_FIRSTHDR(&msg);

     ctl->cmsg_len            = CMSG_LEN(sizeof (int));
     ctl->cmsg_level          = SOL_SOCKET;
     ctl->cmsg_type           = SCM_RIGHTS;
     *((int *)CMSG_DATA(ctl)) = sock_pair[1];

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to establish connection.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     close ( sock_pair[1] );
     close ( sock );
     sock = sock_pair[0];
     

     /* Now we open up a shared memory region */
     bmapfd = shm_open ( SHM_ANON, O_CREAT | O_EXCL | O_RDWR, 00660 );
     if ( bmapfd < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Could not create shared memory region.",
                      "Press any key to abort" );
          close ( sock );
          goto quit;
     }

     if ( ftruncate ( bmapfd, sizeof (uint16_t) * BOARD_SIZE ) < 0 )
     {
          UI_msgbox ( "ERROR",
                      "Could not truncate shared memory",
                      "Press any key to abort" );
          close ( sock );
          goto quit;
     }
     
     bmap = (uint16_t *)mmap ( NULL, sizeof (uint16_t) * BOARD_SIZE,
                               PROT_READ | PROT_WRITE,
                               MAP_NOCORE | MAP_NOSYNC | MAP_SHARED,
                               bmapfd, 0 );

     if ( ((void *)bmap) == MAP_FAILED )
     {
          UI_msgbox ( "ERROR",
                      "Could not instantiate shared memory",
                      "Press any key to abort" );
          close ( sock );
          goto quit;
     }

     memset ( bmap, 0, sizeof (uint16_t) * BOARD_SIZE );


     /* We have a pipe, so we are ready to roll!               */
     /* first ticket out is to register the secretariat number */
     if (!Auto.at)
     {
          sock_path[0] = '\0';
          dbox = UI_cwin ( 5, 35,
                           "Enter Secretariat ID",
                           "Press ENTER to Continue" );
          wattrset ( dbox, A_REVERSE | A_UNDERLINE );
          for ( i = 0; i < 4; i++ )
               mvwaddch ( dbox, 2, (i+2), ' ' );
          UI_prompt ( dbox, 2, 2, sock_path, 4 );
          delwin ( dbox );
          touchwin ( actscr );
          wrefresh ( actscr );
          sid = (uint16_t)strtol ( sock_path, NULL, 16 );
     }
     else
     {
          sid = Auto.sid;
     }

     /* Set up the package */
     if ( init_package ( sid ) < 0 )
          goto quit;

     op_new_reg ( sock, sid );

     /* Now collect some special things */
     /* Dispatch Desk */
     if (!Auto.at)
     {
          strncpy ( sock_path, Auto.cdd, 120 );
          dbox = UI_cwin ( 5, 35,
                           "Enter Dispatch Desk address (word)",
                           "Press ENTER to Continue" );
          wattrset ( dbox, A_REVERSE );
          mvwaddstr ( dbox, 2, 2, "0x" );
          wattron ( dbox, A_UNDERLINE );
          for ( i = 0; i < 6; i++ )
               mvwaddch ( dbox, 4, (i+2), ' ' );
          UI_prompt ( dbox, 2, 4, sock_path, 6 );
          delwin ( dbox );
          touchwin ( actscr );
          wrefresh ( actscr );
          spec_addr = (uint32_t)strtol ( sock_path, NULL, 16 );
     }
     else
     {
          /* We need to seek the address! */
          spec_sym = elf_seek_symbol ( Auto.cdd, sym_list, sym_count );

          if ( !spec_sym )
          {
               printf ( "CDD \"%s\" not found.\n", Auto.cdd );
               exit ( -1 );
          }

          /* Remember, we need a word address */
          spec_addr = (uint16_t)(spec_sym->st_value / 2);

     }

     op_make_xtag ( GEN_DDR, spec_addr, 0 );

     if ( sid > 3 )
     {
          /* Transaction account */
          if (!Auto.at)
          {
               sock_path[0] = '\0';
               strcpy ( sock_path, "0000" );
               dbox = UI_cwin ( 5, 35,
                                "Enter Transaction Account",
                                "Press ENTER to Continue" );
               wattrset ( dbox, A_REVERSE );
               mvwaddstr ( dbox, 2, 2, "0x" );
               wattron ( dbox, A_UNDERLINE );
               for ( i = 0; i < 4; i++ )
                    mvwaddch ( dbox, 4, (i+2), ' ' );
               UI_prompt ( dbox, 2, 4, sock_path, 4 );
               delwin ( dbox );
               touchwin ( actscr );
               wrefresh ( actscr );
               spec_addr = (uint32_t)strtol ( sock_path, NULL, 16 );
          }
          else
          {
               spec_addr = Auto.tx;
          }
          
          if ( spec_addr > 0 )
               op_make_xtag ( GEN_TXA, spec_addr, 0 );

          if (!Auto.at)
          {
               /* ISR */
               sock_path[0] = '\0';
               strcpy ( sock_path, "000000" );
               dbox = UI_cwin ( 5, 35,
                                "Enter ISR address (word)",
                                "Press ENTER to Continue" );
               wattrset ( dbox, A_REVERSE );
               mvwaddstr ( dbox, 2, 2, "0x" );
               wattron ( dbox, A_UNDERLINE );
               for ( i = 0; i < 6; i++ )
                    mvwaddch ( dbox, 4, (i+2), ' ' );
               UI_prompt ( dbox, 2, 4, sock_path, 6 );
               delwin ( dbox );
               touchwin ( actscr );
               wrefresh ( actscr );
               spec_addr = (uint32_t)strtol ( sock_path, NULL, 16 );
          }
          else
          {
               spec_addr = 0;
               
               if ( Auto.isr )
               {
                    spec_sym = elf_seek_symbol
                         ( Auto.isr, sym_list, sym_count );

                    if ( !spec_sym )
                    {
                         printf ( "ISR \"%s\" not found.\n", Auto.cdd );
                         exit ( -1 );
                    }
                    
                    /* Remember, we need a word address */
                    spec_addr = (uint16_t)(spec_sym->st_value / 2);
               }
          }
          
          if ( spec_addr > 0 )
          {
               if (!Auto.at)
               {
                    sock_path[0] = '\0';
                    dbox = UI_cwin ( 5, 35,
                                     "Enter ISR number",
                                     "Press ENTER to Continue" );
                    wattrset ( dbox, A_REVERSE );
                    mvwaddstr ( dbox, 2, 2, "#" );
                    wattron ( dbox, A_UNDERLINE );
                    for ( i = 0; i < 6; i++ )
                         mvwaddch ( dbox, 4, (i+2), ' ' );
                    UI_prompt ( dbox, 2, 3, sock_path, 2 );
                    delwin ( dbox );
                    touchwin ( actscr );
                    wrefresh ( actscr );
                    spec_annx = (uint64_t)strtol ( sock_path, NULL, 10 );
               }
               else
               {
                    spec_annx = (uint64_t)Auto.irq;
               }
               
               op_make_xtag ( GEN_ISR, spec_addr, spec_annx );
          }
     }

     /* Security */
     if (!Auto.at)
     {
          sock_path[0] = '\0';
          strcpy ( sock_path, "ffff" );
          dbox = UI_cwin ( 5, 35,
                           "Enter Downstream Security Level",
                           "Press ENTER to Continue" );
          wattrset ( dbox, A_REVERSE );
          mvwaddstr ( dbox, 2, 2, "0x" );
          wattron ( dbox, A_UNDERLINE );
          for ( i = 0; i < 4; i++ )
               mvwaddch ( dbox, 4, (i+2), ' ' );
          UI_prompt ( dbox, 2, 4, sock_path, 4 );
          delwin ( dbox );
          touchwin ( actscr );
          wrefresh ( actscr );
          ((uint16_t *)&spec_annx)[0] =
               (uint16_t)strtol ( sock_path, NULL, 16 );

          sock_path[0] = '\0';
          strcpy ( sock_path, "0000" );
          dbox = UI_cwin ( 5, 35,
                           "Enter Upstream Security Level",
                           "Press ENTER to Continue" );
          wattrset ( dbox, A_REVERSE );
          mvwaddstr ( dbox, 2, 2, "0x" );
          wattron ( dbox, A_UNDERLINE );
          for ( i = 0; i < 4; i++ )
               mvwaddch ( dbox, 4, (i+2), ' ' );
          UI_prompt ( dbox, 2, 4, sock_path, 4 );
          delwin ( dbox );
          touchwin ( actscr );
          wrefresh ( actscr );
          ((uint16_t *)&spec_annx)[1] =
               (uint16_t)strtol ( sock_path, NULL, 16 );
     }
     else
     {
          ((uint16_t *)&spec_annx)[0] = Auto.dsec;
          ((uint16_t *)&spec_annx)[1] = Auto.usec;
     }
     op_make_xtag ( GEN_SEC, 0, spec_annx );


     /* Base Priority */
     if (!Auto.at)
     {
          sock_path[0] = '\0';
          strcpy ( sock_path, "0000" );
          dbox = UI_cwin ( 5, 35,
                           "Enter Base Priority",
                           "Press ENTER to Continue" );
          wattrset ( dbox, A_REVERSE );
          mvwaddstr ( dbox, 2, 2, "0x" );
          wattron ( dbox, A_UNDERLINE );
          for ( i = 0; i < 4; i++ )
               mvwaddch ( dbox, 4, (i+2), ' ' );
          UI_prompt ( dbox, 2, 4, sock_path, 4 );
          delwin ( dbox );
          touchwin ( actscr );
          wrefresh ( actscr );
          spec_annx = (uint64_t)strtol ( sock_path, NULL, 16 );
     }
     else
     {
          spec_annx = (uint64_t)Auto.bpri;
     }
     op_make_xtag ( GEN_PRI, 0, spec_annx );
          
          
     
     /* Finally, if this is opsec, we will try to do some auto ones */
     /* for FLATLINE/ATRIA */
     if ( !sid )
     {
          spec_sym = elf_seek_symbol ( "sys_flatline", sym_list, sym_count );

          if ( !spec_sym )
          {
               UI_msgbox ( "ERROR",
                           "Invalid OPSEC: no sys_flatline",
                           "Press any key to abort" );
               munmap ( bmap, BOARD_SIZE );
               close ( bmapfd );
               close ( sock );
               goto quit;
          }

          spec_addr = spec_sym->st_value / 2;
          op_make_xtag ( XERIS_FLATLINE, spec_addr, 0 );


          spec_sym = elf_seek_symbol ( "sys_atria", sym_list, sym_count );

          if ( !spec_sym )
          {
               UI_msgbox ( "ERROR",
                           "Invalid OPSEC: no sys_atria",
                           "Press any key to abort" );
               munmap ( bmap, BOARD_SIZE );
               close ( bmapfd );
               close ( sock );
               goto quit;
          }

          spec_addr = spec_sym->st_value / 2;
          op_make_xtag ( XERIS_ATRIA, spec_addr, 0 );
     }
          

     update_tickets ( );

     /* Now we enter into the main dispatching loop        */
     /* We wait for an incoming ticket, and then handle it */
     while ( 1 )
     {
          memset ( &msgvec,   0, sizeof msgvec  );
          memset ( &msg,      0, sizeof msg     );
          memset ( &tic_in,   0, sizeof tic_in  );
               
          msgvec.iov_base          = &tic_in;
          msgvec.iov_len           = sizeof tic_in;
          
          msg.msg_iov              = &msgvec;
          msg.msg_iovlen           = 1;

          i = recvmsg ( sock, &msg, 0 );
          if ( i != sizeof tic_in )
          {
               UI_msgbox ( "ERROR",
                           "Received invalid ticket (1)",
                           "Press any key to abort" );
               munmap ( bmap, BOARD_SIZE );
               close ( bmapfd );
               close ( sock );
               goto quit;
          }

          switch ( tic_in.s )
          {
               case TAG_PRT:
                    op_tag_prt ( sock );
                    break;

               case TAG_ADR:
                    op_tag_adr ( sock );
                    break;

               case TAG_UNI:
                    op_tag_uni ( sock );
                    break;

               case TAG_COM:
                    op_tag_com ( sock );
                    break;

               case TAG_DAT:
                    op_tag_dat ( sock );
                    break;
                    
               case TAG_MAN:
                    op_tag_man ( sock );
                    break;

               case TAG_XFR:
                    op_tag_xfr ( sock );
                    break;

               case 0xffff:
                    /* Our queue to leave! */
                    munmap ( bmap, BOARD_SIZE );
                    close ( bmapfd );
                    close ( sock );
                    goto quit;

               default:
                    UI_msgbox ( "ERROR",
                                "Received invalid ticket",
                                "Press any key to abort" );
                    munmap ( bmap, BOARD_SIZE );
                    close ( bmapfd );
                    close ( sock );
                    goto quit;
          }

          update_tickets ( );
     }
          
               
     

quit:

     if (Auto.at)
          exit ( 0 );

     if ( sym_list )
          free ( sym_list );
     if ( sec_list )
          free ( sec_list );

     delwin ( tic_win_in );
     delwin ( tic_win_out );
     
     flushinp ( );
     getch ( );

     return;
}

     
static void update_ticket ( WINDOW *tw, tag * tic, const char * title )
{
     int may, max, i;

     getmaxyx ( tw, may, max );
     may--; max--;
     
     wclear ( tw );
     box ( tw, 0, 0 );

     wattrset ( tw, A_REVERSE );

     for ( i = 1; i < max; i++ )
          mvwaddch ( tw, 1, i, ' ' );

     UI_cputs ( tw, 1, (char *)title );

     /* Ticket type */
     wattrset ( tw, A_NORMAL );
     mvwaddstr ( tw, 2, 1, "Type" );
     wattrset ( tw, A_BOLD );
     wmove ( tw, 2, 10 );

     switch ( tic->s )
     {
          case XST_MSG:
               waddstr ( tw, "XERIS Message" );
               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "Command" );
               mvwaddstr ( tw, 5, 1, "Argument" );
               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );

               if ( tic->t.msg.ref == NEW_REG )
                         waddstr ( tw, "Register" );
               else
                         waddstr ( tw, "INVALID" );

               wmove ( tw, 5, 10 );
               wprintw ( tw, "%04X", tic->t.msg.arg.a16[0] );

               break;
          
          case TAG_PRT:
               waddstr ( tw, "Partition Tag" );
               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "ident" );
               mvwaddstr ( tw, 5, 1, "word" );
               mvwaddstr ( tw, 6, 1, "align" );
               mvwaddstr ( tw, 7, 1, "ovrly" );
               mvwaddstr ( tw, 8, 1, "extnt" );

               wattrset ( tw, A_REVERSE );
               for ( i = 1; i < max; i++ )
               {
                    mvwaddch ( tw, 10, i, ' ' );
                    mvwaddch ( tw, 13, i, ' ' );
               }

               mvwaddstr ( tw, 10, 1, "Word Size" );
               mvwaddstr ( tw, 13, 1, "Alignment" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "%04X", tic->t.prt.ident );
               
               wmove ( tw, 5, 10 );
               wprintw ( tw, "DAT[%04X]", tic->t.prt.word );
                         
               wmove ( tw, 6, 10 );
               wprintw ( tw, "DAT[%04X]", tic->t.prt.align );

               wmove ( tw, 7, 10 );
               wprintw ( tw, "UNI[%04X]", tic->t.prt.ovrly );

               wmove ( tw, 8, 10 );
               wprintw ( tw, "ADR[%04X]", tic->t.prt.extnt );

               wmove ( tw, 11, 1 );
               wprintw ( tw, "0x%x",
                         reg_pack.dat[tic->t.prt.word].data );

               wmove ( tw, 14, 1 );
               wprintw ( tw, "0x%x",
                         reg_pack.dat[tic->t.prt.align].data );

               
               break;

          case TAG_ADR:
               waddstr ( tw, "Address Tag" );

               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "ident" );
               mvwaddstr ( tw, 5, 1, "part" );
               mvwaddstr ( tw, 6, 1, "segt" );
               mvwaddstr ( tw, 7, 1, "unit" );

               mvwaddstr ( tw, 9, 1, "offs" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "%04X", tic->t.adr.ident );
               
               wmove ( tw, 5, 10 );
               wprintw ( tw, "PRT[%04X]", tic->t.adr.part );
                         
               wmove ( tw, 6, 10 );
               wprintw ( tw, "DAT[%04X]", tic->t.adr.segt );

               wmove ( tw, 7, 10 );
               wprintw ( tw, "UNI[%04X]", tic->t.adr.unit );

               wmove ( tw, 10, 10 );
               wprintw ( tw, "0x%04x",
                         reg_pack.dat[tic->t.adr.offs].data );
               
               break;

          case TAG_UNI:
               waddstr ( tw, "Unit Tag" );
               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "ident" );
               mvwaddstr ( tw, 5, 1, "part" );
               mvwaddstr ( tw, 6, 1, "top" );
               mvwaddstr ( tw, 7, 1, "bot" );
               mvwaddstr ( tw, 8, 1, "com" );

               wattrset ( tw, A_REVERSE );
               for ( i = 1; i < max; i++ )
                    mvwaddch ( tw, 10, i, ' ' );

               mvwaddstr ( tw, 10, 1, "Human Reference" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "%04X", tic->t.uni.ident );
               
               wmove ( tw, 5, 10 );
               wprintw ( tw, "PRT[%04X]",
                         reg_pack.adr[tic->t.uni.top].part );
                         
               wmove ( tw, 6, 10 );
               wprintw ( tw, "0%04x",
                         reg_pack.dat[
                              reg_pack.adr[tic->t.uni.top].offs].data );

               wmove ( tw, 7, 10 );
               wprintw ( tw, "UNI[%04X]",
                         reg_pack.dat[
                              reg_pack.adr[tic->t.uni.bot].offs].data );

               wmove ( tw, 8, 10 );
               wprintw ( tw, "COM[%04X]", tic->t.uni.com );

               if ( tic->t.uni.man > 0 )
                    mvwaddstr ( tw, 11, 1,
                                &man_data[
                                     reg_pack.man[tic->t.uni.man].first] );
               else
                    mvwaddstr ( tw, 11, 1, "----" ); 
                    
               
               break;

          case TAG_COM:
               waddstr ( tw, "Commission Tag" );
               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "ident" );
               mvwaddstr ( tw, 5, 1, "type" );
               mvwaddstr ( tw, 6, 1, "endp" );
               
               wattrset ( tw, A_REVERSE );
               for ( i = 1; i < max; i++ )
               {
                    mvwaddch ( tw, 8, i, ' ' );
                    mvwaddch ( tw, 13, i, ' ' );
               }
               
               mvwaddstr ( tw, 8, 1, "Unit" );
               mvwaddstr ( tw, 13, 1, "Annex" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "%04X", tic->t.com.ident );
               wmove ( tw, 5, 10 );
               switch ( tic->t.com.type )
               {
                    case GEN_MOD:
                         waddstr ( tw, "GEN_MOD" );
                         break;

                    case AVR_ILO:
                         waddstr ( tw, "AVR_ILO" );
                         break;

                    case AVR_IHI:
                         waddstr ( tw, "AVR_IHI" );
                         break;

                    case AVR_INL:
                         waddstr ( tw, "AVR_INL" );
                         break;

                    case AVR_INH:
                         waddstr ( tw, "AVR_INH" );
                         break;

                    case GEN_JMP:
                         waddstr ( tw, "GEN_JMP" );
                         break;

                    case GEN_REL:
                         waddstr ( tw, "GEN_REL" );
                         break;

                    default:
                         waddstr ( tw, "Other" );
                         break;
               }

               wmove ( tw, 6, 10 );
               wprintw ( tw, "%04X",
               reg_pack.dat[reg_pack.adr[tic->t.com.endp].offs].data );

               wmove ( tw, 9, 1 );
               wprintw ( tw, "%04X - %04X",
               reg_pack.dat[reg_pack.adr[reg_pack.uni[tic->t.com.ref].top].offs].data,
               reg_pack.dat[reg_pack.adr[reg_pack.uni[tic->t.com.ref].bot].offs].data);

               mvwaddstr ( tw, 10, 1,
               &man_data[reg_pack.man[reg_pack.uni[tic->t.com.ref].man].first] );

               wmove ( tw, 14, 1 );
               wprintw ( tw, "0x%x", reg_pack.dat[tic->t.com.annx].data );
               
               break;

          case TAG_DAT:
               waddstr ( tw, "Data Tag" );
               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "ident" );
               
               wattrset ( tw, A_REVERSE );
               for ( i = 1; i < max; i++ )
                    mvwaddch ( tw, 6, i, ' ' );
               
               mvwaddstr ( tw, 6, 1, "Payload" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "%04X", tic->t.dat.ident );
               
               wmove ( tw, 7, 1 );
               wprintw ( tw, "0x%x", tic->t.dat.data ); 
               
               break;

          case TAG_XFR:
               waddstr ( tw, "Transfer Slip" );

               wattrset ( tw, A_NORMAL );
               mvwaddstr ( tw, 4, 1, "uni" );
               mvwaddstr ( tw, 5, 1, "seq" );
               mvwaddstr ( tw, 6, 1, "bsz" );

               wattrset ( tw, A_BOLD );
               wmove ( tw, 4, 10 );
               wprintw ( tw, "UNI[%04X]", tic->t.xfr.unit );
               
               wmove ( tw, 5, 10 );
               wprintw ( tw, "%i", tic->t.xfr.seq );
                         
               wmove ( tw, 6, 10 );
               wprintw ( tw, "%i", tic->t.xfr.bsz );

               break;
               

          default:
               waddstr ( tw, "Unknown" );
               break;
     }
                   
                    


     wrefresh ( tw );
     return;
}

static void update_tickets ( void )
{
     if (Auto.at)
          return;
     
     update_ticket ( tic_win_out, &tic_out, "Outbound Ticket" );
     update_ticket ( tic_win_in, &tic_in, "Inbound Ticket" );
     return;
}


static int init_package ( uint16_t sid )
{
     /* This is a big one! */
     long i;
     Elf32_Shdr * sheader;

     uint16_t com_ident;

     /* Set up elf links */
     sec_count = elf_get_sections ( &sec_list );
     sym_count = elf_get_symbols ( &sym_list );



     /* Set up the parition tags first */
     /* Exec parition */
     sheader = elf_seek_section ( ".text", sec_list, sec_count );
     if ( !sheader )
     {
          UI_msgbox ( "ERROR",
                      "Cannot find .text",
                      "Press any key to abort" );
          return ( -1 );
     }


     /* EXE partition setup */

     /* Set up the overlay first */
     reg_pack.uni[uni_free].ident = uni_free;
     
     reg_pack.adr[adr_free].ident = adr_free;
     reg_pack.adr[adr_free].part  = PART_EXE;
     reg_pack.adr[adr_free].segt  = 0;
     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = 0;
     reg_pack.adr[adr_free].offs  = dat_free++;
     reg_pack.adr[adr_free].unit  = uni_free;
     reg_pack.uni[uni_free].top   = adr_free++;

     reg_pack.adr[adr_free].ident = adr_free;
     reg_pack.adr[adr_free].part  = PART_EXE;
     reg_pack.adr[adr_free].segt  = 0;
     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = ((uint64_t)sheader->sh_size / 2) - 1;
     reg_pack.adr[adr_free].offs  = dat_free++;
     reg_pack.adr[adr_free].unit  = uni_free;
     reg_pack.uni[uni_free].bot   = adr_free++;

     reg_pack.uni[uni_free].com   = 0; /* for now */
     reg_pack.uni[uni_free].man   = op_make_man ( ".text" );

     /* Set up the partition tag */
     reg_pack.prt[PART_EXE].ident = PART_EXE;

     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = 16;     
     reg_pack.prt[PART_EXE].word  = dat_free++;

     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = 1;     
     reg_pack.prt[PART_EXE].align = dat_free++;

     reg_pack.prt[PART_EXE].ovrly = uni_free++;

     reg_pack.prt[PART_EXE].extnt =
          reg_pack.adr[
               reg_pack.uni[reg_pack.prt[PART_EXE].ovrly].bot
               ].ident;

     /* OP1 Parition Set-up ("data") */
     sheader = elf_seek_section ( ".data", sec_list, sec_count );
     if ( sheader )
     {
          /* Check the section size. Zero means this */
          /* partition doesn't exist */

          if ( sheader->sh_size > 0 )
          {
               /* Ok. Set up overlay first */
               reg_pack.uni[uni_free].ident = uni_free;

               reg_pack.adr[adr_free].ident = adr_free;
               reg_pack.adr[adr_free].part  = PART_OP1;
               reg_pack.adr[adr_free].segt  = 0;
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 0;
               reg_pack.adr[adr_free].offs  = dat_free++;
               reg_pack.adr[adr_free].unit  = uni_free;
               reg_pack.uni[uni_free].top   = adr_free++;

               reg_pack.adr[adr_free].ident = adr_free;
               reg_pack.adr[adr_free].part  = PART_OP1;
               reg_pack.adr[adr_free].segt  = 0;
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = (uint64_t)sheader->sh_size - 1;
               reg_pack.adr[adr_free].offs  = dat_free++;
               reg_pack.adr[adr_free].unit  = uni_free;
               reg_pack.uni[uni_free].bot   = adr_free++;

               reg_pack.uni[uni_free].com   = 0;
               reg_pack.uni[uni_free].man   = op_make_man ( ".data" );

               /* Partition tag */
               reg_pack.prt[PART_OP1].ident = PART_OP1;
               
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 8;
               reg_pack.prt[PART_OP1].word  = dat_free++;

               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 1;
               reg_pack.prt[PART_OP1].align = dat_free++;

               reg_pack.prt[PART_OP1].ovrly = uni_free++;

               reg_pack.prt[PART_OP1].extnt =
                    reg_pack.adr[
                         reg_pack.uni[reg_pack.prt[PART_OP1].ovrly].bot
                         ].ident;

          }
     }

     /* OP2 Partition Set-up ("bss") */
     sheader = elf_seek_section ( ".bss", sec_list, sec_count );
     if ( sheader )
     {
          /* Check the section size. Zero means this */
          /* partition doesn't exist */

          if ( sheader->sh_size > 0 )
          {
               /* Ok. Set up overlay first */
               reg_pack.uni[uni_free].ident = uni_free;

               reg_pack.adr[adr_free].ident = adr_free;
               reg_pack.adr[adr_free].part  = PART_NUL;
               reg_pack.adr[adr_free].segt  = 0;
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 0;
               reg_pack.adr[adr_free].offs  = dat_free++;
               reg_pack.adr[adr_free].unit  = uni_free;
               reg_pack.uni[uni_free].top   = adr_free++;

               reg_pack.adr[adr_free].ident = adr_free;
               reg_pack.adr[adr_free].part  = PART_NUL;
               reg_pack.adr[adr_free].segt  = 0;
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = (uint64_t)sheader->sh_size - 1;
               reg_pack.adr[adr_free].offs  = dat_free++;
               reg_pack.adr[adr_free].unit  = uni_free;
               reg_pack.uni[uni_free].bot   = adr_free++;

               reg_pack.uni[uni_free].com   = 0;
               reg_pack.uni[uni_free].man   = op_make_man ( ".bss" );

               /* Partition tag */
               reg_pack.prt[PART_OP2].ident = PART_OP2;
               
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 8;
               reg_pack.prt[PART_OP2].word  = dat_free++;

               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = 1;
               reg_pack.prt[PART_OP2].align = dat_free++;

               reg_pack.prt[PART_OP2].ovrly = uni_free++;

               reg_pack.prt[PART_OP2].extnt =
                    reg_pack.adr[
                         reg_pack.uni[reg_pack.prt[PART_OP2].ovrly].bot
                         ].ident;

          }
     }


     /* Now we itterate through all relocation symbols (if any) */
     /* in .text.rela, and assign commission tickets   */
     /* to them */
     sheader = elf_seek_section ( ".rela.text", sec_list, sec_count );
     if ( !sheader )
     {
          rel_count = 0;
          rel_list = NULL;
          goto skip_rela;
     }

     /* For avr, we expect RELA */
     if ( (sheader->sh_type != SHT_RELA) ||
          (sheader->sh_entsize != sizeof (Elf32_Rela)) )
     {
          UI_msgbox ( "ERROR",
                      "Expected RELA section type",
                      "Press any key to abort" );
          return ( -1 );
     }
     
     rel_count = (uint16_t)(sheader->sh_size / sheader->sh_entsize);
     rel_list = (Elf32_Rela *)&memmap[sheader->sh_offset];

     for ( i = 0; i < rel_count; i++ )
     {


          /* We need to intercept some special directives */
          /* if ATRIA is the image being hosted */
          if ( sid == 0x0002 )
          {
               if
               (
                    strcmp
                    (sym_list[rel_list[i].r_info >> 8].name,
                     "atria_secreg_alpha") == 0
               )
               {
                    
                    if ( ((unsigned char)rel_list[i].r_info == R_AVR_LO8_LDI) )
                    {
                         com_ident = op_rela_com ( &rel_list[i] );
                         
                         /* This points to a pair of instructions */
                         /* for loading the SRF alpha block */
                         reg_pack.com[com_ident].type = XERIS_LINK_SRF;
                    }
                    
                    /* Discard otherwise */
                    continue;
               }
               else if
               (
                    strcmp
                    (sym_list[rel_list[i].r_info >> 8].name,
                     "intr_sisr") == 0
               )
               {
                    if ( ((unsigned char)rel_list[i].r_info == R_AVR_LO8_LDI) )
                    {
                         com_ident = op_rela_com ( &rel_list[i] );
                         
                         /* This points to a pair of instructions */
                         /* for loading the SISR root             */
                         reg_pack.com[com_ident].type = XERIS_LINK_SISR;
                    }
                    
                    /* Discard otherwise */
                    continue;
               }

               /* Carry on... */
          }
               
          com_ident = op_rela_com ( &rel_list[i] );

          if ( com_ident <= 0 )
          {
               UI_msgbox ( "ERROR",
                           "Commision Tag generation error",
                           "Press any key to abort" );
               return ( -1 );
          }          

          
          switch ( (unsigned char)rel_list[i].r_info )
          {
               case R_AVR_7_PCREL:
               case R_AVR_13_PCREL:
                    reg_pack.com[com_ident].type = GEN_REL;
                    break;

               case R_AVR_16:
                    reg_pack.com[com_ident].type = GEN_MOD;
                    break;

               case R_AVR_LO8_LDI:
                    reg_pack.com[com_ident].type = AVR_ILO;
                    break;
               case R_AVR_HI8_LDI:
                    reg_pack.com[com_ident].type = AVR_IHI;
                    break;

               case R_AVR_LO8_LDI_NEG:
                    reg_pack.com[com_ident].type = AVR_INL;
                    break;
               case R_AVR_HI8_LDI_NEG:
                    reg_pack.com[com_ident].type = AVR_INH;
                    break;

               case R_AVR_LO8_LDI_GS:
                    reg_pack.com[com_ident].type = AVR_IWL;
                    break;
               case R_AVR_HI8_LDI_GS:
                    reg_pack.com[com_ident].type = AVR_IWH;
                    break;

               case R_AVR_8_LO8:
                    reg_pack.com[com_ident].type = AVR_ML8;
                    break;

               case R_AVR_8_HI8:
                    reg_pack.com[com_ident].type = AVR_MH8;
                    break;

               case R_AVR_CALL:
                    reg_pack.com[com_ident].type = GEN_JMP;
                    break;

               default:
                    UI_msgbox ( "ERROR",
                                "Unsupported relocation type",
                                "Press any key to abort" );
                    return ( -1 );
          }



          if ( i == 0 )
               reg_pack.uni[reg_pack.prt[PART_EXE].ovrly].com = com_ident;
     }

skip_rela:
     
     return ( 0 );
          
     
}


     
static void op_new_reg ( int sock, uint16_t sid )
{
     struct sockaddr_un addr;
     struct iovec msgvec;
     struct msghdr msg;
     struct cmsghdr * ctl;

     union
     {
          char           buf[ CMSG_SPACE(sizeof (int)) ];
          struct cmsghdr align;
     } mapxfer;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );
     memset ( &mapxfer,  0, sizeof mapxfer );

     memset ( &tic_out,  0, sizeof tic_out );

     /* Set-up our ticket first */
     tic_out.s                = XST_MSG;
     tic_out.t.msg.ref        = NEW_REG;
     tic_out.t.msg.arg.a16[0] = sid;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;
     msg.msg_control          = &mapxfer;
     msg.msg_controllen       = sizeof mapxfer;

     ctl                      = CMSG_FIRSTHDR(&msg);

     ctl->cmsg_len            = CMSG_LEN(sizeof (int));
     ctl->cmsg_level          = SOL_SOCKET;
     ctl->cmsg_type           = SCM_RIGHTS;
     *((int *)CMSG_DATA(ctl)) = bmapfd;

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}

static uint16_t op_make_man ( const char * name )
{
     char * new_cursor;
     
     /* Set and pop a new human reference ticket */
     if ( man_cursor >= (MAN_SIZE - 1) )
          return ( 0 );

     if ( man_free >= MAN_POOL )
          return ( 0 );

     new_cursor = stpcpy ( &man_data[man_cursor], name );

     reg_pack.man[man_free].ident = man_free;
     reg_pack.man[man_free].first = man_cursor;
     man_cursor += (uint32_t)(new_cursor - &man_data[man_cursor]);
     reg_pack.man[man_free].last  = man_cursor++;

     return ( man_free++ );
}


static uint16_t op_make_xtag ( uint16_t type, uint32_t addr, uint64_t annx )
{
     /* Basically this is for marking special regions */
     /* such as the dd$, and transaction account, isrs */
     /* and for OPSEC, the location of atria and flatline dd's */

     /* None of the special tags have units associated */
     /* So we just need to add an address tag, sometime */
     /* or an annex sometimes, or both */
     reg_pack.com[com_free].ident = com_free;
     reg_pack.com[com_free].type  = type;
     reg_pack.com[com_free].ref   = 0;

     /* Address? */
     switch ( type )
     {
          case GEN_DDR:
          case GEN_TXA:
          case GEN_ISR:
          case XERIS_FLATLINE:
          case XERIS_ATRIA:
               reg_pack.adr[adr_free].ident = adr_free;

               if ( type == GEN_TXA )
                    /* Tx accounts *must* be initialized */
                    reg_pack.adr[adr_free].part = PART_OP1;
               else
                    reg_pack.adr[adr_free].part = PART_EXE;
               
               reg_pack.adr[adr_free].segt  = 0;

               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = (uint64_t)addr;
               reg_pack.adr[adr_free].offs  = dat_free++;
               reg_pack.adr[adr_free].unit  = 0;
               reg_pack.com[com_free].endp  = adr_free++;
               break;

          default:
               reg_pack.com[com_free].endp  = 0;
               break;
     }

     /* Annex? */
     switch ( type )
     {
          case GEN_MAN:
          case GEN_ISR:
          case GEN_SEC:
          case GEN_PRI:
               reg_pack.dat[dat_free].ident = dat_free;
               reg_pack.dat[dat_free].data  = annx;
               reg_pack.com[com_free].annx  = dat_free++;
               break;

          default:
               reg_pack.com[com_free].annx  = 0;
               break;
     }

     /* Nice */
     return ( com_free++ );
}
     

     
static void op_tag_prt ( int sock )
{
     /* Incoming Parition Tags are always */
     /* Requests for a tag "fill-in"      */
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_PRT;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.prt.ident > 0) && (tic_in.t.prt.ident <= PART_MAN) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.prt, &reg_pack.prt[tic_in.t.prt.ident],
                   sizeof tic_out.t.prt );
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}


static void op_tag_adr ( int sock )
{
     /* Incoming Address Tags are always  */
     /* Requests for a tag "fill-in"      */
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_ADR;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.prt.ident > 0) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.adr, &reg_pack.adr[tic_in.t.adr.ident],
                   sizeof tic_out.t.adr );
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}


static void op_tag_uni ( int sock )
{
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_UNI;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.uni.ident > 0) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.uni, &reg_pack.uni[tic_in.t.uni.ident],
                   sizeof tic_out.t.uni );
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}


static void op_tag_com ( int sock )
{
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_COM;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.com.ident > 0) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.com, &reg_pack.com[tic_in.t.com.ident],
                   sizeof tic_out.t.com );
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}


static void op_tag_dat ( int sock )
{
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_DAT;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.dat.ident > 0) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.dat, &reg_pack.dat[tic_in.t.dat.ident],
                   sizeof tic_out.t.dat );
     }
     else
     {
          /* ident 0x0000 - return test pattern */
          tic_out.t.dat.data = 0xFEDCBA9876543210;
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }
     
     return;

}


static void op_tag_man ( int sock )
{
     struct iovec msgvec;
     struct msghdr msg;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     tic_out.s                = TAG_MAN;

     /* Set up the datagram */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;

     /* Check for valid range */
     /* If invalid, we send a "blank" tag back */
     if ( (tic_in.t.com.ident > 0) )
     {
          /* Good to go, we just copy it in directly */
          memcpy ( &tic_out.t.man, &reg_pack.man[tic_in.t.man.ident],
                   sizeof tic_out.t.man );
          return;
     }

     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }

     return;

}

static void op_tag_xfr ( int sock )
{
     struct iovec msgvec;
     struct msghdr msg;

     uint32_t temp_bot;

     Elf32_Shdr * sheader;

     memset ( &msgvec,   0, sizeof msgvec  );
     memset ( &msg,      0, sizeof msg     );

     memset ( &tic_out,  0, sizeof tic_out );

     memset ( (void *)bmap, 0, (BOARD_SIZE * 2) );

     /* Check if this is in sequence, and matches the unit */
     if ( (tic_in.t.xfr.seq != act_xfr.seq) )
     {
          UI_msgbox ( "ERROR",
                      "Transfer out of sequence",
                      "Press any key to abort" );
          act_xfr.unit  = 0;
          act_xfr.seq   = 0;
          act_xfr_total = 0;
          act_xfr_sent  = 0;
          act_xfr_mark  = 0;
          return;
     }

     /* If this is sequence zero, set up */
     if ( act_xfr.seq == 0 )
     {

          /* There are two possible partitions which this load */
          /* can be from. Either PART_EXE (.text) or PART_OP1  */
          /* (.data) */

          /* The board itself is configured for 16-bit words,       */
          /* so in the case of OP1 we need to pre-convert to 16-bit */

          

          
          act_xfr.unit = tic_in.t.xfr.unit;

          act_xfr_mark =
               reg_pack.dat[
                    reg_pack.adr
                    [reg_pack.uni[act_xfr.unit].top].offs]
               .data;

          temp_bot =
               reg_pack.dat[
                    reg_pack.adr
                    [reg_pack.uni[act_xfr.unit].bot].offs]
               .data;
          
          /* Check only OP1, because OP2 is for (.bss), and so */
          /* is invalid in out implmention */
          if ( (reg_pack.adr[reg_pack.uni[act_xfr.unit].top].part
                == PART_OP1) )
          {
               /* Pre-adjust to 16-bit word address */
               temp_bot /= 2;
          }

          
          act_xfr_total =
               temp_bot - act_xfr_mark + 1;

          
          /* Note that act_xfr_mark should be a byte address */
          /* when we are actually indexing into the elf      */
          act_xfr_mark *= 2;
          
          /* Add in the internal offset from the partition */
          switch ( reg_pack.adr[
                        reg_pack.uni[act_xfr.unit].top].part )
          {

               case PART_EXE:
                    /* Add the .text offset to word_mark */
                    
                    sheader = elf_seek_section ( ".text", sec_list, sec_count );
                    if ( !sheader )
                    {
                         UI_msgbox ( "ERROR",
                                     "Cannot find .text",
                                     "Press any key to abort" );
                         return;
                    }
                    break;

               case PART_OP1:
                    /* Add the .data offset to word_mark */
                    
                    sheader = elf_seek_section ( ".data", sec_list, sec_count );
                    if ( !sheader )
                    {
                         UI_msgbox ( "ERROR",
                                     "Cannot find .data",
                                     "Press any key to abort" );
                         return;
                    }
                    break;

               default:
                    UI_msgbox ( "ERROR",
                                "Transfer partition invalid",
                                "Press any key to abort" );
                    act_xfr.unit  = 0;
                    act_xfr.seq   = 0;
                    act_xfr_total = 0;
                    act_xfr_sent  = 0;
                    act_xfr_mark  = 0;
                    return;
          }
          
          act_xfr_mark += (uint32_t)sheader->sh_offset;
          
     }
     
     if ( act_xfr.unit != tic_in.t.xfr.unit )
     {
          UI_msgbox ( "ERROR",
                      "Transfer out of sequence",
                      "Press any key to abort" );
          act_xfr.unit  = 0;
          act_xfr.seq   = 0;
          act_xfr_total = 0;
          act_xfr_sent  = 0;
          act_xfr_mark  = 0;
          return;
     }


     /* Ok, now we send the next part in the sequence */
     msgvec.iov_base          = &tic_out;
     msgvec.iov_len           = sizeof tic_out;

     msg.msg_iov              = &msgvec;
     msg.msg_iovlen           = 1;


     tic_out.s          = TAG_XFR;

     tic_out.t.xfr.unit = act_xfr.unit;
     tic_out.t.xfr.seq  = act_xfr.seq;

     /* Calculate how much we can fit */
     if ( (act_xfr_total - act_xfr_sent) > BOARD_SIZE )
     {
          /* We can't fit it all on this one */
          act_xfr_sent += BOARD_SIZE;

          tic_out.t.xfr.bsz = BOARD_SIZE;

          memcpy ( (void *)bmap,
                   &memmap[act_xfr_mark],
                   BOARD_SIZE * 2 );

          act_xfr_mark += (BOARD_SIZE * 2);
          act_xfr.seq++;
     }
     else
     {
          /* This is the last ticket */
          tic_out.t.xfr.bsz = (act_xfr_total - act_xfr_sent);
          
          memcpy ( (void *)bmap,
                   &memmap[act_xfr_mark],
                   tic_out.t.xfr.bsz * 2 );

          act_xfr.unit  = 0;
          act_xfr.seq   = 0;
          act_xfr_total = 0;
          act_xfr_sent  = 0;
          act_xfr_mark  = 0;
     }


     if ( sendmsg ( sock, &msg, 0 ) < sizeof tic_out )
     {
          UI_msgbox ( "ERROR",
                      "Unable to dispatch ticket.",
                      "Press any key to continue" );
          memset ( &tic_out,  0, sizeof tic_out );
     }
     
     return;
          
}


static uint16_t op_rela_com ( Elf32_Rela *rela )
{

     /* Rela entries convert quite conveniently into Commission  */
     /* Tags. The only sticking point is setting the address     */
     /* tags to the correct parition. In the case of AVR, if     */
     /* the address masked with 0x0080 0000, it indicates that   */
     /* it must be in one of the Operational Partitions. Luckily */
     /* such addresses only appear in associated symbols, which  */
     /* also indicate the associated section                     */

     /* Also note that for AVR, the ram offsets are included, so */
     /* we need to compensate for that in those cases            */

     /* Note that we're only going to process Rela entries in    */
     /* .text.rela. So we know that the Rela->r_offset is always */
     /* in the Executive Partition */

     Elf32_Word sym_idx = (rela->r_info >> 8);
     uint16_t det_part;
     
     reg_pack.com[com_free].ident = com_free;
     
     reg_pack.adr[adr_free].ident = adr_free;
     reg_pack.adr[adr_free].part  = PART_EXE;
     reg_pack.adr[adr_free].segt  = 0;

     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = (uint64_t)rela->r_offset / 2;
     reg_pack.adr[adr_free].offs  = dat_free++;
     reg_pack.adr[adr_free].unit  = 0;
     reg_pack.com[com_free].endp  = adr_free++;

     /* Detect partition */
     if ( (sym_idx > 0) )
     {
          if ( sym_list[sym_idx].sym->st_shndx < SHN_LORESERVE )
          {
               if ( !strncmp
                    ( sec_list[sym_list[sym_idx].sym->st_shndx].name,
                      ".text", 5 ) )
                    det_part = PART_EXE;
               else if ( !strncmp
                         ( sec_list[sym_list[sym_idx].sym->st_shndx].name,
                           ".data", 5 ) )
                    det_part = PART_OP1;
               else if ( !strncmp
                         ( sec_list[sym_list[sym_idx].sym->st_shndx].name,
                           ".bss", 4 ) )
                    det_part = PART_OP2;
               else
               {
                    UI_msgbox ( "ERROR",
                                "Invalid symbol section",
                                "Press any key to continue" );
                    return ( -1 );
               }
          }
          else if ( sym_list[sym_idx].sym->st_shndx == SHN_ABS )
          {
               det_part = PART_EXE;
          }
          else
          {
               UI_msgbox ( "ERROR",
                           "Invalid symbol section type",
                           "Press any key to continue" );
               return ( -1 );
          }

     }
     else
     {
          det_part = PART_NUL;
     }
     
     /* Encode symbol */
     reg_pack.uni[uni_free].ident = uni_free;

     reg_pack.adr[adr_free].ident = adr_free;
     reg_pack.adr[adr_free].part  = det_part;
     reg_pack.adr[adr_free].segt  = 0;

     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = (uint64_t)
          (sym_list[sym_idx].sym->st_value);

     if ( reg_pack.dat[dat_free].data > 0 )
     {
          if ( det_part == PART_EXE )
               reg_pack.dat[dat_free].data /= 2;
          else if ( det_part != PART_NUL )
          {
               reg_pack.dat[dat_free].data &= (uint64_t)0xffff;
               reg_pack.dat[dat_free].data -= (uint64_t)OP_MIN;
          }
     }
     reg_pack.adr[adr_free].offs  = dat_free++;
     reg_pack.adr[adr_free].unit  = uni_free;
     reg_pack.uni[uni_free].top   = adr_free++;

     reg_pack.adr[adr_free].ident = adr_free;
     reg_pack.adr[adr_free].part  = det_part;
     reg_pack.adr[adr_free].segt  = 0;

     reg_pack.dat[dat_free].ident = dat_free;
     reg_pack.dat[dat_free].data  = (uint64_t)
          (sym_list[sym_idx].sym->st_value +
           (sym_list[sym_idx].sym->st_size > 0 ?
            sym_list[sym_idx].sym->st_size : 1)
           - 1);
     if ( reg_pack.dat[dat_free].data > 0 )
     {
          if ( det_part == PART_EXE )
               reg_pack.dat[dat_free].data /= 2;
          else if ( det_part != PART_NUL )
          {
               reg_pack.dat[dat_free].data &= (uint64_t)0xffff;
               reg_pack.dat[dat_free].data -= (uint64_t)OP_MIN;
          }
     }
     
     reg_pack.adr[adr_free].offs  = dat_free++;
     reg_pack.adr[adr_free].unit  = uni_free;
     reg_pack.uni[uni_free].bot   = adr_free++;

     reg_pack.uni[uni_free].com   = com_free;
     reg_pack.uni[uni_free].man   =
          op_make_man ( (const char *)sym_list[sym_idx].name );
     reg_pack.com[com_free].ref   = uni_free++;

     if ( rela->r_addend > 0 )
     {
          reg_pack.dat[dat_free].ident = dat_free;
          reg_pack.dat[dat_free].data  = (uint64_t)rela->r_addend;


          /* Note, we need to adjust this based on  */
          /* the detected partition. If the partition */
          /* is EXE, the addend is a byte address, but */
          /* on the ticket, it needs to be a word value */
          if ( det_part == PART_EXE )
               reg_pack.dat[dat_free].data /= 2;

          reg_pack.com[com_free].annx  = dat_free++;
     }
     else
     {
          reg_pack.com[com_free].annx = 0;
     }


     return ( com_free++ );

}
