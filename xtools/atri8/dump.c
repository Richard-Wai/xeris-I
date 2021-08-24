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

#include "globl.h"
#include "ui.h"
#include "elf.h"
#include "dump.h"


/*

    The dump screen!

*/

long dump_init ( void )
{
     int dwiny, dwinx, bpl, bpp, hexb, bin;
     long i, j, ppg;
     char buf[18];

     win_adr  = 0;
     sel_adr  = 0;
     sel_size = 1;

     dumpwin =
          derwin
          (
               dumpscr,
               ((termy + 1) - 6), ((termx + 1) - 2),
               4, 1
          );

     if ( !dumpwin )
          err_nomem ( );

     getmaxyx ( dumpwin, dwiny, dwinx );
     dwiny--, dwinx--;

     /* Who doesn't have 80 characters? */
     if ( dwinx < 70 )
     {
          delwin ( dumpwin );
          dumpwin =
               UI_cwin
               (
                    6, (dwinx - 4),
                    "Error",
                    "Press any Key ..."
               );

          if ( !dumpwin )
               err_nomem ( );

          UI_cputs ( dumpwin, 2, "Your terminal is too narrow." );
          
          flushinp ( );
          wgetch ( dumpwin );
          delwin ( dumpwin );

          return ( 0 );
     }
     
     wattrset ( dumpscr, A_NORMAL );
     wattrset ( dumpwin, A_NORMAL );
     wclear ( dumpscr );
     
     /* 16 bytes per line, how many lines? */
     bpp = ( 16 * ( dwiny + 1 ) );

     /* find the number of pages needed */
     ppg = max_adr / (long)bpp;

     /* Always add two more */
     ppg += 2;
     
     /* create the pad */
     dumppad = newpad ( ((dwiny + 1) * ppg), (dwinx + 1) );
     if ( !dumppad )
          err_nomem ( );

     wclear ( dumppad );
     wattrset ( dumppad, A_NORMAL );
     
     UI_fwin
     (
          dumpscr, "HEX Analysis",
          ""
     );

     // print out the offset header
     wattrset ( dumpscr, A_BOLD );
     /* 8 for address, +2 for spacing */
     wmove ( dumpscr, 2, 11 );
     for ( i = 0; ( i < 16 ); i++ )
          wprintw ( dumpscr, "+%01x ", i );
     wattrset ( dumpscr, A_NORMAL );

     /* Dump the memory map */

     for ( j = 0, i = 0; j < max_adr; j += 16 )
     {
          wmove ( dumppad, (j/16), 0 );
          wattrset ( dumppad, A_BOLD );
          wprintw ( dumppad, "%08x  ", j );
          wattrset ( dumppad, A_NORMAL );

          
          for ( i = 0; i < 16; i++ )
          {
               if ( (j + i) > max_adr )
               {
                    wprintw ( dumppad, "   " );
                    buf[i] = ' ';
               }
               else
               {
                    bin = (int)memmap[j+i];
                    wprintw ( dumppad, "%02x ", bin );
                    if ( isprint ( bin ) )
                         buf[i] = (char)bin;
                    else
                         buf[i] = '.';
               }

          }
          
          buf[i] = '\0';
          wprintw ( dumppad, "   %s", buf );
     }

     return ( max_adr );
}

void dump_kill ( void )
{
     delwin ( dumpwin );
     delwin ( dumppad );
}

void dump_base ( void )
{
     int may, max, key, idx, i;
     long tmp;
     char inp[128];

     /* The dump screen! */
     actscr = dumpscr;

     if ( !sel_ref )
          sel_ref = sel_list;

     getmaxyx ( actscr, may, max );
     may--, max--;

     /* Set-up status bar */

     wmove ( actscr, may, 0 );
     wclrtoeol ( actscr );
     wattrset ( actscr, A_REVERSE );
     for ( i = 0; i <= max; i++ )
          mvwaddch ( actscr, may, i, ' ' );
     mvwaddstr
     (
          actscr, may, 1,
          "(?) Command List"
     );

     wrefresh ( actscr );

     dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );

     while ( 1 )
     {

          /* Print selection address and size boxes */
          wattrset ( actscr, A_REVERSE );
          wmove ( actscr, 2, 1 );
          wprintw ( actscr, "%08x", sel_adr );

          if ( sel_size == 1 )
               wattrset ( actscr, A_NORMAL );
          
          wmove ( actscr, 2, 62 );
          for ( i = 0; i < 16; i++ )
               waddch ( actscr, ' ' );

          if ( sel_size > 1 )
          {
               wmove ( actscr, 2, 62 );
               wprintw ( actscr, "%i Bytes", sel_size );
               wattrset ( actscr, A_NORMAL );
          }

          wattrset ( actscr, A_REVERSE );

          /* Update status bar */
          for ( i = 0; i <= max; i++ )
               mvwaddch ( actscr, may, i, ' ' );

          /* Reference selector */
          wattrset ( actscr, A_STANDOUT );
          for ( i = 1; i <= 30 ; i++ )
               mvwaddch ( actscr, may, i, ' ' );

          wmove ( actscr, may, 1 );
          wprintw ( actscr, " [%s]", sel_ref->name );

          wattrset ( actscr, A_BOLD | A_REVERSE );
          wmove ( actscr, may, 41 );
          wprintw ( actscr, "%+i", (sel_adr - sel_ref->adr) );

          dump_refresh ( win_adr );
          wattrset ( actscr, A_REVERSE );
          mvwaddstr ( actscr, may, max - 31,
                      "Press (?) for Command Listing" );
          wrefresh ( actscr );
          
          flushinp ( );

          key = getch ( );

          if ( key == '?' )
               key = dump_commen ( );

          touchwin ( actscr );
          wrefresh ( actscr );
                    

          switch ( key )
          {
               case KEY_RIGHT:
                    if ( ( sel_adr + (sel_size * 2)) > max_adr )
                         break;

                    dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );

                    sel_adr += sel_size;
                    if ( (sel_adr + sel_size) >
                         dump_limit ( win_adr ) )
                         win_adr = sel_adr;

                    dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
                    break;
                    

               case KEY_LEFT:
                    if ( (sel_adr - sel_size) < 0 )
                         break;

                    dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
                    
                    sel_adr -= sel_size;
                    if ( sel_adr < ((win_adr/16) * 16) )
                    {
                         win_adr = 16 + (sel_adr -
                              ((dump_limit(win_adr)) - win_adr));

                         if ( win_adr < 0 )
                              win_adr = 0;
                    }

                    dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
                    break;

               case KEY_DOWN:
                    if ( dump_limit ( win_adr ) > max_adr )
                         break;

                    win_adr += 16;
                    break;

               case KEY_UP:
                    win_adr -= 16;

                    if ( win_adr < 0 )
                         win_adr = 0;

                    break;

               case KEY_PPAGE:
                    win_adr -= (dump_limit ( win_adr ) - win_adr);
                    if ( win_adr < 0 )
                         win_adr = 0;

                    break;

               case KEY_NPAGE:
                    if ( dump_limit ( win_adr ) >= max_adr )
                         break;

                    win_adr = (dump_limit ( win_adr ) + 1);
                    break;

               case 'h':
               case 'H':
                    win_adr = sel_adr;
                    dump_centre ( &win_adr );
                    break;

               case 'e':
               case 'E':
                    /* Edit region */
                    dump_select ( &win_adr, &sel_adr, &sel_size );
                    break;

               case 'q':
               case 'Q':
                    return;

               case KEY_ENTER:
               case KEY_RETURN:
               case 'x':
               case 'X':
                    dump_refexec ( );
                    break;

               case 'r':
               case 'R':
               case ' ':
                    dump_refop ( );
                    break;

               case 'g':
               case 'G':
                    /* Goto address */
                    wattrset ( actscr, A_UNDERLINE );
                    mvwaddstr ( actscr, 2, 1, "        " );

                    inp[0] = '\0';

                    wrefresh  ( actscr );
                    UI_prompt ( actscr, 2, 1, inp, 8 );

                    tmp = strtol ( inp, NULL, 16 );
                    if ( tmp > max_adr )
                    {
                         beep ();
                         break;
                    }
                         
                    dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
                    sel_adr = tmp;
                    dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
                    win_adr = tmp;
                    break;

               case KEY_TAB:
                    if ( sel_ref->next )
                         sel_ref = sel_ref->next;
                    else
                         sel_ref = sel_list;
                    
                    dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
                    sel_adr = sel_ref->adr;
                    sel_size = sel_ref->size;
                    dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
                    win_adr = sel_adr;
                    break;
                    

               default:
                    beep ( );
                    break;
          }


     }

                    
     return;
}
     

void dump_refresh ( long nadr )
{
    static long adr;        // current address

    int dmay, dmax;
    int cmay, cmax;


    getmaxyx ( dumpwin, dmay, dmax );
    dmay--, dmax--;

    getmaxyx ( dumppad, cmay, cmax );
    cmay--;

    if ( nadr >= 0 )
        adr = nadr;

    /* Seek to the provided address, highlight indexes and */
    /* hex value, and place cursor at corresponding ascii  */
    /* dump */
    
    copywin ( dumppad, dumpwin, (adr / 16), 0, 0, 0, dmay, dmax, FALSE );

    touchwin ( dumpscr );
    wrefresh ( dumpscr );
    wrefresh ( dumpwin );
    

    return;
}

void dump_centre ( long * adr )
{
     /* Centre the dump window at the address */
     int dmay, dmax;

     getmaxyx ( dumpwin, dmay, dmax );
     dmay--, dmax--;
     

     *adr -= (dmay / 2) * 16;
     if ( *adr < 0 )
          *adr = 0;

     dump_refresh ( *adr );
     return;
}

void dump_select ( long * dump_adr, long * sel_adr, int * size )
{

     int dmay, dmax, smay, smax;
     long cursor_adr = *sel_adr;

     WINDOW *savebar;

     register int i;

     getmaxyx ( dumpwin, dmay, dmax );
     dmay--, dmax--;

     getmaxyx ( dumpscr, smay, smax );
     smay--, smax--;

     /* Save status bar */
     savebar = newpad ( 1, (smax + 1) );
     if ( !savebar )
          err_nomem ( );

     copywin ( dumpscr, savebar, smay, 0, 0, 0, 0, smax, FALSE );

     if
     (
          (*sel_adr < *dump_adr) ||
          (*sel_adr > (*dump_adr + (dmax * 16)))
     )
     {
          /* Selection is outside of view, centre view */
          *dump_adr = *sel_adr + (1 * (*size / 16));
          dump_centre ( dump_adr );
     }

     /* Enter out main "event loop" */
     while ( 1 )
     {
          /* Set statusbar and do selection */
          wattrset ( dumpscr, A_REVERSE );
          for ( i = 0; i <= smax; i++ )
               mvwaddch ( dumpscr, smay, i, ' ' );

          wmove   ( dumpscr, smay, 1 );
          wattron ( dumpscr, A_BOLD );
          waddstr ( dumpscr, "Edit Region   " );
          wattroff ( dumpscr, A_BOLD );
          
          mvwaddstr
          ( dumpscr, smay, (smax - 48),
          "Use arrow keys to adjust. Press RETURN to accept." );
          wprintw ( dumpscr, "     " );

          /* Update selection/size */
          wmove ( actscr, 2, 1 );
          wprintw ( dumpscr, "%08x", *sel_adr );

          if ( *size == 1 )
               wattrset ( dumpscr, A_NORMAL );
          
          wmove ( actscr, 2, 62 );
          for ( i = 0; i < 16; i++ )
               waddch ( actscr, ' ' );

          if ( *size > 1 )
          {
               wmove ( dumpscr, 2, 62 );
               wprintw ( dumpscr, "%i Bytes", *size );
               wattrset ( dumpscr, A_NORMAL );
          }
          
          dump_refresh ( *dump_adr );
          dump_highlight ( UI_UNHIGHLIGHT, *sel_adr, *size );
          flushinp ();

          switch ( getch () )
          {
               case KEY_UP:
                    (*size) -= 16;
                    break;

               case KEY_DOWN:
                    (*size) += 16;
                    break;

               case KEY_RIGHT:
                    (*size)++;
                    break;

               case KEY_LEFT:
                    (*size)--;
                    break;

               case KEY_ENTER:
               case KEY_RETURN:
               case 'q':
               case 'Q':
                    dump_highlight ( UI_HIGHLIGHT, *sel_adr, *size );
                    copywin
                    ( savebar, dumpscr, 0, 0, smay, 0, smay, smax, FALSE );
                    wrefresh ( dumpscr );

                    delwin ( savebar );
                    return;

               default:
                    break;
          }


          
          if ( *size < 1 )
               *size = 1;          
                    
          if ( (*sel_adr + *size) > max_adr )
               *size = max_adr - *sel_adr;

          dump_highlight ( UI_HIGHLIGHT, *sel_adr, *size );
     }

     return;
}

     

void dump_highlight ( unsigned char mode, long adr, int size )
{
     /* Add highlight to address indicated, both on dump page */
     /* and dump window */

     unsigned char lowbyte = ((unsigned char)adr & 0x0f);
     register int i;

     int seek_row, seek_col, get_row, get_col;
     char buf[8];

     if ( mode == UI_HIGHLIGHT )
     {
          wattrset ( dumpscr, A_UNDERLINE | A_BOLD );
          wattrset ( dumppad, A_UNDERLINE );
     }
     else
     {
          wattrset ( dumpscr, A_BOLD );
          wattrset ( dumppad, A_NORMAL );
     }

     /* First do the index highlights */
     seek_row = 2;
     seek_col = 11 + (3 * lowbyte);
     wmove ( dumpscr, seek_row, seek_col );
     waddch ( dumpscr, (char)(winch ( dumpscr ) & A_CHARTEXT) );
     waddch ( dumpscr, (char)(winch ( dumpscr ) & A_CHARTEXT) );

     /* Find the ultimate address line */
     seek_col = 0;
     seek_row = adr / 16;
     wmove ( dumppad, seek_row, seek_col );

     wattron ( dumppad, A_BOLD );
     for ( i = 0; i < 8; i++ )
          waddch ( dumppad, (char)(winch ( dumppad ) & A_CHARTEXT) );

     if ( mode == UI_HIGHLIGHT )
          wattrset ( dumppad, A_REVERSE );
     else
          wattrset ( dumppad, A_NORMAL );

     /* Now do the hex byte highlight */
     seek_col = 10 + (3 * lowbyte);

     wmove ( dumppad, seek_row, seek_col );
     
     for ( i = 1; i <= size; i++ )
     {
          waddch ( dumppad, (char)(winch ( dumppad ) & A_CHARTEXT) );
          waddch ( dumppad, (char)(winch ( dumppad ) & A_CHARTEXT) );

          getyx ( dumppad, get_row, get_col );

          if ( (size > 1) && ( i < size) )
               if ( (get_col > 11) && (get_col < (12+(15*3))) )
                    /* add spacing */
                    waddch ( dumppad, (char)(winch ( dumppad ) & A_CHARTEXT) );
          
          if ( get_col > 55 )
          {
               /* End of row! do a wrap */
               seek_row++;
               seek_col = 10;
               wmove ( dumppad, seek_row, seek_col );
               continue;
          }

     }

     /* Now do the ascii highlight */
     seek_row = adr / 16;
     seek_col = 61 + lowbyte;

     wmove ( dumppad, seek_row, seek_col );

     for ( i = 0; i < size; i++ )
     {
          waddch ( dumppad, (char)(winch ( dumppad ) & A_CHARTEXT) );

          getyx ( dumppad, get_row, get_col );

          if ( get_col >= 77 )
          {
               /* End of row! do a wrap */
               seek_row++;
               seek_col = 61;
               wmove ( dumppad, seek_row, seek_col );
          }
     }

     return;
}

long dump_limit ( long adr )
{
     /* Bottom limit */
     int may, max;
     getmaxyx ( dumpwin, may, max );
     may--,max--;

     return ( (((adr/16) + may) * 16) + 15 );
}


int dump_commen ( void )
{
     /* Show command listing */
     WINDOW * win;
     struct MenuItemList menu[13];
     int i;

     /* Load menu */
     for ( i = 0; i < 13; i++ )
     {
          menu[i].item = hex_cmd_list[i];
          menu[i].key  = hex_cmd_keys[i];
     }

     win = UI_cwin
     (
          18, 40,
          "Command Listing",
          "Select a Command"
     );

     i = UI_static_menu ( win, 2, menu, 12, 1 );

     if ( (i == 'q') || (i == 'Q') )
          i = 0;
     
     delwin ( win );
     return ( i );
}

void dump_info_eheader ( void )
{
    WINDOW *ifowin, *menwin, *menpad;
    int i, j, thisy, thisx, menrow, mencol, pay, pax, cmd;
    char *msgbuf;
    char exbuf[80];

    struct Selection * sel_finger;

    #define IFOOF 24            // alignment of info column
    const int menmax = 18;      // last element of the menu

    memcpy
    (
         (void *)&eheader,
         (void *)&memmap[sel_adr],
         sizeof ( Elf32_Ehdr )
    );

    // set-up window based on size of terminal
    ifowin = UI_cwin
    (
         (LINES - 6 ), (COLS / 2) + 6,
         "ELF Header", "Press ENTER to act on item"
    );
    touchwin ( ifowin );
    wrefresh ( ifowin );

    // get size of provided window
    getmaxyx ( ifowin, thisy, thisx );
    thisy--; thisx--;

    // set-up actual menu area subwindow (derived window)
    menwin =
         derwin
         ( ifowin, ( thisy - 3 ), ( thisx - 4 ), 2, 2 );

    if ( !menwin )
        err_nomem ( );

    // set-up menu buffer (pad) based on menu area
    getmaxyx ( menwin, menrow, mencol );
    menrow--, mencol--;
    menpad = newpad ( ( menmax + 1 ), (mencol+1) );
    if ( !menpad )
        err_nomem ( );

    /* Clear the pad */
    getmaxyx ( menpad, pay, pax );
    pay--, pax--;
    
    wattrset ( menpad, A_REVERSE );
    for ( i = 0; i <= pay; i++ )
         for ( j = 0; j <= pax; j++ )
              mvwaddch ( menpad, i, j, ' ' );

    for ( i = 0; i <= menmax; i++ )
        // place all the lables into the menu buffer
        mvwaddstr ( menpad, i, 0, menu_info_header[i] );

    // check elf header magic
    if
    (
        ( eheader.e_ident[EI_MAG0] == ELFMAG0 ) &&
        ( eheader.e_ident[EI_MAG1] == ELFMAG1 ) &&
        ( eheader.e_ident[EI_MAG2] == ELFMAG2 ) &&
        ( eheader.e_ident[EI_MAG3] == ELFMAG3 )
    )
        msgbuf = "[OK]";
    else
        msgbuf = "[NG]";

    mvwaddstr ( menpad, 0,                      IFOOF, msgbuf );
    // Output header size
    sprintf ( exbuf, "%i bytes", (unsigned int)eheader.e_ehsize );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 1,                      IFOOF, msgbuf );

    // class
    switch ( eheader.e_ident[EI_CLASS] )
    {
        case ELFCLASSNONE:
            msgbuf = "NONE (Invalid)";
            break;
        case ELFCLASS32:
            msgbuf = "32 Bit";
            break;
        case ELFCLASS64:
            msgbuf = "64 Bit";
            break;
        default:
            msgbuf = "Unknown";
    }
    sprintf ( exbuf, "%s (0x%x)", msgbuf, eheader.e_ident[EI_CLASS] );
    msgbuf = exbuf;

    mvwaddstr ( menpad, 2,                      IFOOF, msgbuf );

    // encoding
    switch ( eheader.e_ident[EI_DATA] )
    {
        case ELFDATANONE:
            msgbuf = "NONE (Invalid)";
            break;
        case ELFDATA2LSB:
            msgbuf = "Little endian";
            break;
        case ELFDATA2MSB:
            msgbuf = "Big endian";
            break;
        default:
            msgbuf = "Unknown";
    }
    mvwaddstr ( menpad, 3,                      IFOOF, msgbuf );

    // version in header ident field
    if ( eheader.e_ident[EI_VERSION] == eheader.e_version )
        msgbuf = "[OK]";
    else
        msgbuf = "[Mismatch]";

    sprintf ( exbuf, "%s (%i)", msgbuf, (unsigned int)eheader.e_ident[EI_DATA] );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 4,                      IFOOF, msgbuf );

    // "Type"
    switch ( eheader.e_type )
    {
        case ET_NONE:
            msgbuf = "None";
            break;
        case ET_REL:
            msgbuf = "Relocatable";
            break;
        case ET_EXEC:
            msgbuf = "Executable";
            break;
        case ET_DYN:
            msgbuf = "Dynamic link";
            break;
        case ET_CORE:
            msgbuf = "Core dump";
            break;
        case ET_LOPROC:
            msgbuf = "Proc (LO)";
            break;
        case ET_HIPROC:
            msgbuf = "Proc (HI)";
            break;
        default:
            msgbuf = "UNKNOWN";
    }
    mvwaddstr ( menpad, 5, IFOOF, msgbuf );

    // Architecture
    sprintf ( exbuf, "0x%x", eheader.e_machine );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 6, IFOOF, msgbuf );

    // ELF version
    sprintf ( exbuf, "%i", (unsigned int)eheader.e_version );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 7, IFOOF, msgbuf );

    // Entry address
    sprintf ( exbuf, "0x%x", eheader.e_entry );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 8, IFOOF, msgbuf );

    // Flags
    sprintf ( exbuf, "0x%x", eheader.e_flags );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 9, IFOOF, msgbuf );

    // Program header offset
    sprintf ( exbuf, "0x%x", eheader.e_phoff );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 12, IFOOF, msgbuf );

    // program header entires and size
    sprintf ( exbuf, "%i x %i bytes", (int)eheader.e_phnum, (int)eheader.e_phentsize );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 13, IFOOF, msgbuf );

    // Section header offset
    sprintf ( exbuf, "0x%x", eheader.e_shoff );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 16, IFOOF, msgbuf );

    // Section header entires and size
    sprintf ( exbuf, "%i x %i bytes", (int)eheader.e_shnum, (int)eheader.e_shentsize );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 17, IFOOF, msgbuf );

    // Section string index
    sprintf ( exbuf, "0x%x", eheader.e_shstrndx );
    msgbuf = exbuf;
    mvwaddstr ( menpad, 18, IFOOF, msgbuf );



    // Copy menu contents to "selected" pad for UI_domenu
    switch ( UI_domenu ( menwin, menpad, 0 ) )
    {
         case 11:
              /* Program header */
              /* Attempt to insert new entry */

              /* Seek to end */
              sel_finger = sel_list;
              while ( sel_finger->next )
                   sel_finger = sel_finger->next;
              
              sel_finger->next =
                   (struct Selection *)
                   malloc ( sizeof ( struct Selection ) );
              
              if ( !sel_finger->next )
                   err_nomem ( );

              /* set as active reference */
              sel_ref = sel_finger->next;

              /* Set-up name */
              sel_ref->name = malloc ( sizeof ( char ) * SEL_NAMEMAX );
              if ( !sel_ref->name )
                   err_nomem ( );

              /* Copy-over name */
              strncpy
              (
                   sel_ref->name,
                   sel_type_list[SEL_PHEADER],
                   SEL_NAMEMAX
              );

              /* Set-up entry */
              sel_ref->prev = sel_finger;
              sel_ref->next = NULL;
              sel_ref->adr  = (long)eheader.e_phoff;
              sel_ref->size = (int)eheader.e_phentsize;
              sel_ref->type = SEL_PHEADER;

              /* Select as  */
              
              dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
              sel_adr  = sel_ref->adr;
              sel_size = sel_ref->size;
              dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
              win_adr = sel_adr;
              break;

         case 15:
              /* Section header */
              /* Attempt to insert new entry */

              /* Seek to end */
              sel_finger = sel_list;
              while ( sel_finger->next )
                   sel_finger = sel_finger->next;
              
              sel_finger->next =
                   (struct Selection *)
                   malloc ( sizeof ( struct Selection ) );
              
              if ( !sel_finger->next )
                   err_nomem ( );

              /* set as active reference */
              sel_ref = sel_finger->next;

              /* Set-up name */
              sel_ref->name = malloc ( sizeof ( char ) * SEL_NAMEMAX );
              if ( !sel_ref->name )
                   err_nomem ( );

              /* Copy-over name */
              strncpy
              (
                   sel_ref->name,
                   sel_type_list[SEL_SHEADER],
                   SEL_NAMEMAX
              );

              /* Set-up entry */
              sel_ref->prev = sel_finger;
              sel_ref->next = NULL;
              sel_ref->adr  = (long)eheader.e_shoff;
              sel_ref->size = (int)eheader.e_shentsize;
              sel_ref->type = SEL_SHEADER;

              /* Select as  */
              
              dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
              sel_adr  = sel_ref->adr;
              sel_size = sel_ref->size;
              dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
              win_adr = sel_adr;
              break;

         default:
              break;
    }

    delwin ( menwin );
    delwin ( menpad );
    delwin ( ifowin );

    return;

}


void dump_info_pheader ( void )
{
     WINDOW *ifowin, *menwin, *menpad;
     int i, j, may, max, thisy, thisx, menrow, mencol, pay, pax, cmd;
     int preflag;
     char *msgbuf;
     char exbuf[80];

     struct Selection * sel_finger;
     Elf32_Phdr pheader;

     
     const int menmax = 7;      /* last element of the menu        */
     const int rescol = 20;     /* Colum offset of interpretations */

     /* Get current terminal size for reference */
     getmaxyx ( actscr, may, max );
     may--, max--;

     memcpy
     (
          (void *)&pheader,
          (void *)&memmap[sel_adr],
          sizeof ( Elf32_Phdr )
     );

     
     /* set-up window based on size of terminal */
     ifowin = UI_cwin
     (
          ((menmax + 1) + 4), (max / 2) + 2,
          "Program Header", "Press ENTER to act on item"
     );
     touchwin ( ifowin );
     wrefresh ( ifowin );

     /* get size of provided window */
     getmaxyx ( ifowin, thisy, thisx );
     thisy--; thisx--;

     /* set-up actual menu area subwindow (derived window) */
     menwin =
          derwin
          ( ifowin, ( thisy - 3 ), ( thisx - 3 ), 2, 2 );

     if ( !menwin )
          err_nomem ( );

     /* set-up menu buffer (pad) based on menu area */
     getmaxyx ( menwin, menrow, mencol );
     menrow--, mencol--;
     menpad = newpad ( ( menmax + 1 ), (mencol+1) );
     if ( !menpad )
          err_nomem ( );

     /* Clear the pad */
     getmaxyx ( menpad, pay, pax );
     pay--, pax--;

     /* "Clear" the menu pad */
     wattrset ( menpad, A_REVERSE );
     for ( i = 0; i <= pay; i++ )
          for ( j = 0; j <= pax; j++ )
               mvwaddch ( menpad, i, j, ' ' );

     for ( i = 0; i <= menmax; i++ )
          /* place all the lables into the menu buffer */
          mvwaddstr ( menpad, i, 0, info_pheader[i] );

     /* Interpret the entry type */
     if ( (pheader.p_type >= 0) && (pheader.p_type <= 6) )
          mvwaddstr
          (
               menpad, 0, rescol,
               info_pheader_type[pheader.p_type]
          );
     else
          mvwaddstr
          (
               menpad, 0, rescol,
               info_pheader_type[7]
          );

     /* Offset */
     wmove ( menpad, 1, rescol );
     wprintw ( menpad, "%08x", pheader.p_offset );

     /* Virtual Address */
     wmove ( menpad, 2, rescol );
     wprintw ( menpad, "%08x", pheader.p_vaddr );

     /* Physical Address */
     wmove ( menpad, 3, rescol );
     wprintw ( menpad, "%08x", pheader.p_paddr );

     /* File Size */
     wmove ( menpad, 4, rescol );
     wprintw ( menpad, "%i Bytes", pheader.p_filesz );

     /* Memory Size */
     wmove ( menpad, 5, rescol );
     wprintw ( menpad, "%i Bytes", pheader.p_memsz );

     /* Flags! */
     preflag = 0;
     wmove ( menpad, 6, rescol );

     if ( pheader.p_flags & PF_R )
     {
          waddch ( menpad, 'R' );
          preflag = 1;
     }

     if ( pheader.p_flags & PF_W )
     {
          if ( preflag )
               waddstr ( menpad, " + " );
          waddch ( menpad, 'W' );
          preflag = 1;
     }

     if ( pheader.p_flags & PF_X )
     {
          if ( preflag )
               waddstr ( menpad, " + " );
          waddch ( menpad, 'X' );
          preflag = 1;
     }

     /* Last, and also least, alignment */
     wmove ( menpad, 7, rescol );
     wprintw ( menpad, "%i", pheader.p_align );

     
     switch ( UI_domenu ( menwin, menpad, 0 ) )
     {
          case 1:
               /* Selected offset, we will simply move */
               /* the active selection there.. */
               /* First check if the file-size is zero */
               /* if it is, there is no actual data in this file */
               /* to even look at, so do nothing */
               if ( pheader.p_filesz == 0 )
               {
                    UI_msgbox
                    (
                         "Conceptual Section",
                         "This section has no data",
                         "Press any key to continue"
                    );
                    break;
               }
               
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)pheader.p_offset;
               sel_size = (int)pheader.p_filesz;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

         default:
              break;
    }

    delwin ( menwin );
    delwin ( menpad );
    delwin ( ifowin );

    return;

}

void dump_info_sheader ( void )
{
     WINDOW *ifowin, *menwin, *menpad;
     int i, j, may, max, thisy, thisx, menrow, mencol, pay, pax, cmd;
     int preflag;
     long strtab_adr;
     unsigned char *sname;
     char exbuf[80];

     struct Selection * sel_finger;
     Elf32_Shdr sheader, strtab;

     
     const int menmax = 9;      /* last element of the menu         */
     const int rescol = 20;     /* Column offset of interpretations */

     /* Get current terminal size for reference */
     getmaxyx ( actscr, may, max );
     may--, max--;

     memcpy
     (
          (void *)&sheader,
          (void *)&memmap[sel_adr],
          sizeof ( Elf32_Shdr )
     );

     
     /* set-up window based on size of terminal */
     ifowin = UI_cwin
     (
          ((menmax + 1) + 4), (max / 2) + 2,
          "Section Header", "Press ENTER to act on item"
     );
     touchwin ( ifowin );
     wrefresh ( ifowin );

     /* get size of provided window */
     getmaxyx ( ifowin, thisy, thisx );
     thisy--; thisx--;

     /* set-up actual menu area subwindow (derived window) */
     menwin =
          derwin
          ( ifowin, ( thisy - 3 ), ( thisx - 3 ), 2, 2 );

     if ( !menwin )
          err_nomem ( );

     /* set-up menu buffer (pad) based on menu area */
     getmaxyx ( menwin, menrow, mencol );
     menrow--, mencol--;
     menpad = newpad ( ( menmax + 1 ), (mencol+1) );
     if ( !menpad )
          err_nomem ( );

     /* Clear the pad */
     getmaxyx ( menpad, pay, pax );
     pay--, pax--;

     /* "Clear" the menu pad */
     wattrset ( menpad, A_REVERSE );
     for ( i = 0; i <= pay; i++ )
          for ( j = 0; j <= pax; j++ )
               mvwaddch ( menpad, i, j, ' ' );

     for ( i = 0; i <= menmax; i++ )
          /* place all the lables into the menu buffer */
          mvwaddstr ( menpad, i, 0, info_sheader[i] );

     /* Attempt to access and load the name */
     /* First, verify that it gets anywhere.. */

     strtab_adr = (long)(eheader.e_shoff +
                         (eheader.e_shstrndx *
                          eheader.e_shentsize));
     
     /* Note, according to ELF format, if there is no strtab */
     /* then it will point to the null entry SHN_UNDEF (0)   */
     /* just in case.. */
     if ( (long)(strtab_adr + eheader.e_shentsize) > max_adr )
          strtab_adr = 0;

     
     /* Load in string table entry */
     memcpy
     (
          (void *)&strtab,
          (void *)&memmap[strtab_adr],
          sizeof ( Elf32_Shdr )
     );
     
     if
     (
          (sheader.sh_name > 0) &&
          ((strtab.sh_offset + sheader.sh_name) < max_adr)
     )
     {
          sname = &memmap
          [
               (long)
               (strtab.sh_offset + sheader.sh_name)
          ];
          
          /* We will only load the first 20 */
          for ( i = 0; i < 20; i++ )
          {

               if
               (
                    (strtab.sh_offset + sheader.sh_name + i) >
                    max_adr
               )
                    break;
               
               if ( !sname[i] )
                    break;

               mvwaddch ( menpad, 0, (rescol + i), sname[i] );
          }
     }
     else
          sname = NULL;
     
     /* Interpret the section type */
     if ( (sheader.sh_type >= 0) && (sheader.sh_type <= 11) )
          mvwaddstr
          (
               menpad, 1, rescol,
               info_sheader_type[sheader.sh_type]
          );
     else
          mvwaddstr
          (
               menpad, 1, rescol,
               info_sheader_type[12]
          );

     /* Flags! */
     preflag = 0;
     wmove ( menpad, 2, rescol );

     if ( sheader.sh_flags & SHF_WRITE )
     {
          waddstr ( menpad, "WRITE" );
          preflag = 1;
     }

     if ( sheader.sh_flags & SHF_ALLOC )
     {
          if ( preflag )
               waddstr ( menpad, "/" );
          waddstr ( menpad, "ALLOC" );
          preflag = 1;
     }

     if ( sheader.sh_flags & SHF_EXECINSTR )
     {
          if ( preflag )
               waddstr ( menpad, "/" );
          waddstr ( menpad, "INSTR" );
          preflag = 1;
     }

     if ( sheader.sh_flags & SHF_MASKPROC )
     {
          if ( preflag )
               waddstr ( menpad, "/" );
          waddstr ( menpad, "PROC" );
     }
     
     /* Address */
     wmove ( menpad, 3, rescol );
     wprintw ( menpad, "0x%08x", sheader.sh_addr );

     /* Section Table Offset */
     wmove ( menpad, 4, rescol );
     wprintw ( menpad, "0x%08x", sheader.sh_offset );

     /* Section Table Size */
     wmove ( menpad, 5, rescol );
     wprintw ( menpad, "0x%08x", sheader.sh_size );

     /* Link Offset */
     wmove ( menpad, 6, rescol );
     wprintw ( menpad, "0x%08x", sheader.sh_link );

     /* Info offset */
     wmove ( menpad, 7, rescol );
     wprintw ( menpad, "0x%08x", sheader.sh_info );

     /* Align */
     wmove ( menpad, 8, rescol );
     wprintw ( menpad, "%i", sheader.sh_addralign );

     /* Section Table Entry Size */
     wmove ( menpad, 9, rescol );
     wprintw ( menpad, "%i Bytes", sheader.sh_entsize );

     if ( sheader.sh_offset > max_adr )
     {
          UI_msgbox
          (
               "Error",
               "Offset out of range.",
               "Pree any key to continue"
          );
          
          delwin ( menwin );
          delwin ( menpad );
          delwin ( ifowin );
          return;
     }
          
     switch ( UI_domenu ( menwin, menpad, 0 ) )
     {
          case 0:
               /* Check if we should do a spawn */
               if ( sel_adr != sel_ref->adr )
                    dump_spawn ( );
               
               /* Auto rename reference */
               snprintf
               (
                    sel_ref->name,
                    sizeof ( char ) * SEL_NAMEMAX,
                    "Section \"%s\"",
                    sname ? sname : (unsigned char *)""
               );
               break;
                    
               
          case 1:
               /* Add entry type refrence */
               dump_info_sheader_doref ( &sheader, sname, sel_adr );
               break;
          
          case 4:
               /* Select address */
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)sheader.sh_offset;
               sel_size = 1;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          case 5:
               /* Select region (section size) */
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)sheader.sh_offset;
               sel_size = (int)sheader.sh_size;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          case 6:
               /* Select Link offset */
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)sheader.sh_link;
               sel_size = 1;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          case 7:
               /* Select Info offset */
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)sheader.sh_info;
               sel_size = 1;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          case 9:
               /* Select First Entry */
               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = (long)sheader.sh_offset;
               sel_size = (int)sheader.sh_entsize;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

         default:
              break;
    }

    delwin ( menwin );
    delwin ( menpad );
    delwin ( ifowin );

    return;

}

void dump_info_sheader_doref
( Elf32_Shdr * sheader, unsigned char *sname, long shaddr )
{
     struct Selection * sel_finger;

     unsigned char seltype;
     int selsize;
     
     /* Put this in it's own area to do all the various */
     /* refrence operations depending on the section    */
     /* header type.. */

     /* Make new entry */
     sel_finger = sel_list;
     while ( sel_finger->next )
          sel_finger = sel_finger->next;
              
     sel_finger->next =
          (struct Selection *)
          malloc ( sizeof ( struct Selection ) );
              
     if ( !sel_finger->next )
          err_nomem ( );

     /* set as active reference */
     sel_ref = sel_finger->next;

     /* Select the appropriate type */
     switch ( sheader->sh_type )
     {
          case SHT_SYMTAB:
               seltype = SEL_SYMBOL;
               selsize = (int)sizeof ( Elf32_Sym );
               break;

          case SHT_STRTAB:
               seltype = SEL_STRING;
               selsize = (int)sheader->sh_entsize;
               break;

          case SHT_REL:
               seltype = SEL_REL;
               selsize = (int)sizeof ( Elf32_Rel );
               break;

          case SHT_RELA:
               seltype = SEL_RELA;
               selsize = (int)sizeof ( Elf32_Rela );
               break;

          case SHT_PROGBITS:
               seltype = SEL_MECH;
               selsize = (int)sheader->sh_size;
               break;

          default:
               seltype = SEL_DEFAULT;
               selsize = (int)sheader->sh_entsize;
               break;
     }

     /* Set up type appropriately */


     /* Set up selection region */
     /* First, check the entry size and selection size */
     
     if ( selsize != (int)sheader->sh_entsize )
          UI_msgbox
          (
               "Warning",
               "Entry Size and Entry Type Disagree",
               "Press a key to continue"
          );

     if ( selsize < 1 )
          selsize = 1;

     /* Copy-over name */
     sel_ref->name = malloc ( sizeof ( char ) * SEL_NAMEMAX );
     if ( !sel_ref->name )
          err_nomem ( );

     /* Set-up name */
     snprintf
     (
          sel_ref->name,
          sizeof ( char ) * SEL_NAMEMAX,
          "%s (%s)",
          sel_type_list[seltype],
          sname ? sname : (unsigned char *)""
     );
     
     /* Set-up entry */
     sel_ref->prev = sel_finger;
     sel_ref->next = NULL;
     sel_ref->adr  = (long)sheader->sh_offset;
     sel_ref->size = (int)selsize;
     sel_ref->ref  = shaddr;
     sel_ref->type = seltype;

     /* Lastly, set the selector */
     dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
     sel_adr  = sel_ref->adr;
     sel_size = sel_ref->size;
     dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
     win_adr = sel_adr;

     return;
}


/* Symbol table entry and reloc info */
void dump_info_symtab ( void )
{
     WINDOW *ifowin, *menwin, *menpad;
     int i, j, may, max, thisy, thisx, menrow, mencol, pay, pax, cmd;
     int preflag;
     long strtab_adr, check_adr;
     unsigned char symchar, *sname;
     char exbuf[80];

     struct Selection * sel_finger;
     Elf32_Shdr * sheader, * strtab;
     Elf32_Sym * symbol;

     
     const int menmax = 7;      /* last element of the menu         */
     const int rescol = 20;     /* Column offset of interpretations */


     sname = NULL;

     /* Waste no time! */
     symbol = (Elf32_Sym *)&memmap[sel_adr];
     

     /* We're going to need to reference the symbol string section */
     /* The offset to the actual string region should be in the    */
     /* global variable "symstroff". If it isnt, we will load it   */
     if ( !symstroff )
          elf_seek_symstroff ( );

     /* Also, we're going to need to load the associated section   */
     /* header, as well as the section header string table section */
     /* header so that we can look-up the name! */

     /* Ensure eheader is ok */
     if ( eheader.e_shentsize != sizeof ( Elf32_Shdr ) )
     {
          UI_msgbox
          (
               "ERROR",
               "ELF Header: Bad Section Header Size",
               "Pree any key to cancel"
          );
          return;
     }

     /* Load Parent Section Header (if not "special") */

     if ( symbol->st_shndx < SHN_LORESERVE )
     {
          check_adr =
               eheader.e_shoff +
               (eheader.e_shentsize * symbol->st_shndx);
     
          if ( check_adr > max_adr )
               sname = NULL;
          else
               sheader = (Elf32_Shdr *)&memmap[check_adr];
     }
     else
     {
          switch ( symbol->st_shndx )
          {
               case SHN_LOPROC:
                    sname = "[LOPROC]";
                    break;

               case SHN_HIPROC:
                    sname = "[HIPROC]";
                    break;

               case SHN_ABS:
                    sname = "[ABSOLUTE]";
                    break;

               case SHN_COMMON:
                    sname = "[COMMON]";
                    break;

               case SHN_HIRESERVE:
                    sname = "[HIRESERVE]";
                    break;

               default:
                    sname = "[INVALID]";
                    break;
          }
     }

     if ( sname )
          goto no_section;
     
     /* Section String Table */
     check_adr =
          eheader.e_shoff +
          (eheader.e_shstrndx * eheader.e_shentsize);

     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "ELF Header: Bad Section String Table",
               "Pree any key to cancel"
          );
          return;
     }

     strtab = (Elf32_Shdr *)&memmap[check_adr];


     if ( (strtab->sh_offset + sheader->sh_name) > max_adr )
     {
          UI_msgbox
          (
               "INVALID",
               "ELF Header: Bad Section Name String",
               "Pree any key to cancel"
          );
          return;
     }
     sname = &memmap[strtab->sh_offset + sheader->sh_name];

 no_section:
     
     /* OK, everything is "loaded" so let's set everything up!     */
     
     /* Get current terminal size for reference */
     getmaxyx ( actscr, may, max );
     may--, max--;      
    
     /* set-up window based on size of terminal */
     ifowin = UI_cwin
     (
          ((menmax + 1) + 4), (max / 2) + 2,
          "Symbol", "Press ENTER to act on item"
     );
     touchwin ( ifowin );
     wrefresh ( ifowin );

     /* get size of provided window */
     getmaxyx ( ifowin, thisy, thisx );
     thisy--; thisx--;

     /* set-up actual menu area subwindow (derived window) */
     menwin =
          derwin
          ( ifowin, ( thisy - 3 ), ( thisx - 3 ), 2, 2 );

     if ( !menwin )
          err_nomem ( );

     /* set-up menu buffer (pad) based on menu area */
     getmaxyx ( menwin, menrow, mencol );
     menrow--, mencol--;
     menpad = newpad ( ( menmax + 1 ), (mencol+1) );
     if ( !menpad )
          err_nomem ( );

     /* Clear the pad */
     getmaxyx ( menpad, pay, pax );
     pay--, pax--;

     /* "Clear" the menu pad */
     wattrset ( menpad, A_REVERSE );
     for ( i = 0; i <= pay; i++ )
          for ( j = 0; j <= pax; j++ )
               mvwaddch ( menpad, i, j, ' ' );

     for ( i = 0; i <= menmax; i++ )
          /* place all the lables into the menu buffer */
          mvwaddstr ( menpad, i, 0, info_symtab[i] );

     /* Symbol name */
     /* We will only load the first 20 */

     for ( i = 0; i < 20; i++ )
     {
          if
          (
               (symstroff + symbol->st_name + i) >
               max_adr
          )
               break;

          symchar = memmap[symstroff + symbol->st_name + i];
               
          if ( !symchar )
               break;

          mvwaddch ( menpad, 0, (rescol + i), symchar );
     }

     /* Value */
     wmove ( menpad, 1, rescol );
     wprintw ( menpad, "0x%08x", symbol->st_value );

     /* Size */
     wmove ( menpad, 2, rescol );
     wprintw ( menpad, "0x%08x", symbol->st_size );

     /* Binding */
     wmove ( menpad, 3, rescol );
     switch ( symbol->st_info >> 4 )
     {
          case STB_LOCAL:
               waddstr ( menpad, "LOCAL" );
               break;

          case STB_GLOBAL:
               waddstr ( menpad, "GLOBAL" );
               break;

          case STB_WEAK:
               waddstr ( menpad, "WEAK" );
               break;

          case STB_LOPROC:
               waddstr ( menpad, "LOPROC" );
               break;

          case STB_HIPROC:
               waddstr ( menpad, "HIPROC" );
               break;

          default:
               waddstr ( menpad, "(invalid)" );
               break;
     }

     /* Type */
     wmove ( menpad, 4, rescol );
     switch ( symbol->st_info & 0x0f )
     {
          case STT_NOTYPE:
               waddstr ( menpad, "NOTYPE" );
               break;

          case STT_OBJECT:
               waddstr ( menpad, "OBJECT" );
               break;

          case STT_FUNC:
               waddstr ( menpad, "FUNC" );
               break;

          case STT_SECTION:
               waddstr ( menpad, "SECTION" );
               break;

          case STT_FILE:
               waddstr ( menpad, "FILE" );
               break;

          case STT_LOPROC:
               waddstr ( menpad, "LOPROC" );
               break;

          case STT_HIPROC:
               waddstr ( menpad, "HIPROC" );
               break;

          default:
               waddstr ( menpad, "(invalid)" );
               break;
     }

     /* "other field" */
     wmove ( menpad, 5, rescol );
     wprintw ( menpad, "0x%02x ", symbol->st_other );
     if ( !symbol->st_other )
          waddstr ( menpad, "(Normal)" );
     else
          waddstr ( menpad, "(Unexpected)" );

     /* Section name */
     /* We will only load the first 20 */
     if ( !sname )
          sname = "[NO ASSOICATION]";
     
     for ( i = 0; i < 20; i++ )
     {
          if ( !sname[i] )
               break;

          mvwaddch ( menpad, 6, (rescol + i), sname[i] );
     }

     /* Section index */
     wmove ( menpad, 7, rescol );
     wprintw ( menpad, "%i", symbol->st_shndx );
          
     switch ( UI_domenu ( menwin, menpad, 0 ) )
     {
         default:
              break;
     }

    delwin ( menwin );
    delwin ( menpad );
    delwin ( ifowin );

    return;

}



/* Symbol table entry and reloc info */
void dump_info_reloc ( void )
{
     WINDOW *ifowin, *menwin, *menpad;
     int i, j, may, max, thisy, thisx, menrow, mencol, pay, pax, cmd;
     int preflag;
     long strtab_adr, check_adr, addend;
     unsigned char symchar, *symsecname, *unitsecname;
     char exbuf[80];

     struct Selection * sel_finger;
     Elf32_Shdr *symsheader, *strtab, *unitsheader;
     Elf32_Sym *symbol;

     Elf32_Rel *rel;

     const int rescol = 20;     /* Column offset of interpretations */
     const int menmax = 14;

     symsecname = NULL;
     unitsecname = NULL;
     
     /* Is reloc or rela? */
     rel = (Elf32_Rel *)&memmap[sel_adr];
     
     if ( sel_size == sizeof ( Elf32_Rel ) )
     {
          addend = 0;
     }
     else if ( sel_size == sizeof ( Elf32_Rela ) )
     {
          addend = (long)
               ( ((Elf32_Rela *)&memmap[sel_adr])->r_addend );
     }
     else
     {
          UI_msgbox
          (
               "ERROR",
               "Not a valid Relocation Entry",
               "Pree any key to cancel"
          );
          return;
     }

     
     /* Going to need these ... */
     if ( !symstroff )
          elf_seek_symstroff ( );

     /* Also, we're going to need to load the associated section   */
     /* header, as well as the section header string table section */
     /* header so that we can look-up the name! */

     /* Ensure eheader is ok */
     if ( eheader.e_shentsize != sizeof ( Elf32_Shdr ) )
     {
          UI_msgbox
          (
               "ERROR",
               "ELF Header: Bad Section Header Size",
               "Pree any key to cancel"
          );
          return;
     }

     
     /* First we need to load in the parent relocation section */
     /* header */
     check_adr = sel_ref->ref;
     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "Bad section header reference",
               "Pree any key to cancel"
          );
          return;
     }

     unitsheader = (Elf32_Shdr *)&memmap[check_adr];

     /* Now we can load the associated symbol section header */
     check_adr =
          eheader.e_shoff +
          (eheader.e_shentsize * unitsheader->sh_link);
          
     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "Bad relocation symtab reference",
               "Pree any key to cancel"
          );
          return;
     }
     symsheader = (Elf32_Shdr *)&memmap[check_adr];
     
     /* The parent sectionheader gives an index (sh_info) for */
     /* the actual "unit" section header, which is the section */
     /* header for the section into which the actual relocation */
     /* unit is offset */
     check_adr =
          eheader.e_shoff +
          (eheader.e_shentsize * unitsheader->sh_info);

     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "Bad relocation section reference",
               "Pree any key to cancel"
          );
          return;
     }
     unitsheader = (Elf32_Shdr *)&memmap[check_adr];



     /* Now we need to look-up the index of the refrence symbol */
     check_adr =
          symsheader->sh_offset +
          ( ((long)(rel->r_info >> 8)) *
            sizeof ( Elf32_Sym ) );

     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "Reference symbol index invalid",
               "Pree any key to cancel"
          );
          return;
     }
     
     symbol = (Elf32_Sym *)&memmap[check_adr];


     /* Load Parent Section Header, unless it is a "special" */
     /* section */
     if ( symbol->st_shndx < SHN_LORESERVE )
     {
          check_adr =
               eheader.e_shoff +
               (eheader.e_shentsize * symbol->st_shndx);
     
          if ( check_adr > max_adr )
               symsheader = NULL;
          else
               symsheader = (Elf32_Shdr *)&memmap[check_adr];
               
     }
     else
     {
          switch ( symbol->st_shndx )
          {
               case SHN_LOPROC:
                    symsecname = "[LOPROC]";
                    break;

               case SHN_HIPROC:
                    symsecname = "[HIPROC]";
                    break;

               case SHN_ABS:
                    symsecname = "[ABSOLUTE]";
                    break;

               case SHN_COMMON:
                    symsecname = "[COMMON]";
                    break;

               case SHN_HIRESERVE:
                    symsecname = "[HIRESERVE]";
                    break;

               default:
                    symsecname = "[INVALID]";
                    break;
          }
     }
     

     
     /* Section String Table */
     check_adr =
          eheader.e_shoff +
          (eheader.e_shstrndx * eheader.e_shentsize);

     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "ELF Header: Bad Section String Table",
               "Pree any key to cancel"
          );
          return;
     }

     strtab = (Elf32_Shdr *)&memmap[check_adr];

 
     /* Symbol section name */
     if ( (!symsecname) && (symsheader) )
     {
          if ( (strtab->sh_offset + symsheader->sh_name) > max_adr )
          {
               UI_msgbox
               (
                    "INVALID",
                    "ELF Header: Bad Symbol Section Name String",
                    "Pree any key to cancel"
                    );
               return;
          }
          
          symsecname = &memmap[strtab->sh_offset + symsheader->sh_name];
     }
     
     /* Relocation unit section name */
     if ( (strtab->sh_offset + unitsheader->sh_name) > max_adr )
     {
          UI_msgbox
          (
               "INVALID",
               "ELF Header: Bad Relocation Section Name String",
               "Pree any key to cancel"
          );
          return;
     }
     unitsecname = &memmap[strtab->sh_offset + unitsheader->sh_name];


 
     /* OK, everything is "loaded" so let's set everything up!     */
     
     /* Get current terminal size for reference */
     getmaxyx ( actscr, may, max );
     may--, max--;      
    
     /* set-up window based on size of terminal */
     ifowin = UI_cwin
     (
          ((menmax + 1) + 4), (max / 2) + 2,
          "Relocation Entry", "Press ENTER to act on item"
     );
     touchwin ( ifowin );
     wrefresh ( ifowin );

     /* get size of provided window */
     getmaxyx ( ifowin, thisy, thisx );
     thisy--; thisx--;

     /* set-up actual menu area subwindow (derived window) */
     menwin =
          derwin
          ( ifowin, ( thisy - 3 ), ( thisx - 3 ), 2, 2 );

     if ( !menwin )
          err_nomem ( );

     /* set-up menu buffer (pad) based on menu area */
     getmaxyx ( menwin, menrow, mencol );
     menrow--, mencol--;
     menpad = newpad ( ( menmax + 1 ), (mencol+1) );
     if ( !menpad )
          err_nomem ( );

     /* Clear the pad */
     getmaxyx ( menpad, pay, pax );
     pay--, pax--;

     /* "Clear" the menu pad */
     wattrset ( menpad, A_REVERSE );
     for ( i = 0; i <= pay; i++ )
          for ( j = 0; j <= pax; j++ )
               mvwaddch ( menpad, i, j, ' ' );

     for ( i = 0; i <= menmax; i++ )
          /* place all the lables into the menu buffer */
          mvwaddstr ( menpad, i, 0, info_reloc[i] );



     /* Unit's section name */
     /* We will only load the first 20 */
     if ( !unitsecname )
          unitsecname = "[INVALID]";

     
     for ( i = 0; i < 20; i++ )
     {
          if ( !unitsecname[i] )
               break;

          mvwaddch ( menpad, 0, (rescol + i), unitsecname[i] );
     }

     
     /* Relocation offset */
     wmove ( menpad, 1, rescol );
     wprintw ( menpad, "0x%08x", rel->r_offset );

     /* Addend */
     wmove ( menpad, 2, rescol );
     if ( addend )
          wprintw ( menpad, "0x%08x", addend );
     else
          waddstr ( menpad, "NO ADDEND" );

     /* Type: */
     /* For now, avr only */
     wmove ( menpad, 3, rescol );
     if ( (unsigned char)rel->r_info > R_AVR_MAXTYPES )
          waddstr ( menpad, "[INVALID]" );
     else
          waddstr ( menpad, reloc_avr[(unsigned char)rel->r_info] );

     /* Index */
     wmove ( menpad, 4, rescol );
     wprintw ( menpad, "%i", (long)(rel->r_info >> 8) );

     /* Symbol name */
     /* We will only load the first 20 */
     for ( i = 0; i < 20; i++ )
     {
          if ( (symstroff + symbol->st_name + i) > max_adr )
               break;

          symchar =
               (unsigned char)memmap[symstroff + symbol->st_name + i];
          
          if ( !symchar )
               break;

          mvwaddch ( menpad, 7, (rescol + i), symchar );
     }

     /* Value */
     wmove ( menpad, 8, rescol );
     wprintw ( menpad, "0x%08x", symbol->st_value );

     /* Size */
     wmove ( menpad, 9, rescol );
     wprintw ( menpad, "0x%08x", symbol->st_size );
     
     /* Binding */
     wmove ( menpad, 10, rescol );
     switch ( symbol->st_info >> 4 )
     {
          case STB_LOCAL:
               waddstr ( menpad, "LOCAL" );
               break;

          case STB_GLOBAL:
               waddstr ( menpad, "GLOBAL" );
               break;

          case STB_WEAK:
               waddstr ( menpad, "WEAK" );
               break;

          case STB_LOPROC:
               waddstr ( menpad, "LOPROC" );
               break;

          case STB_HIPROC:
               waddstr ( menpad, "HIPROC" );
               break;

          default:
               waddstr ( menpad, "(invalid)" );
               break;
     }

     /* Type */
     wmove ( menpad, 11, rescol );
     switch ( symbol->st_info & 0x0f )
     {
          case STT_NOTYPE:
               waddstr ( menpad, "NOTYPE" );
               break;

          case STT_OBJECT:
               waddstr ( menpad, "OBJECT" );
               break;

          case STT_FUNC:
               waddstr ( menpad, "FUNC" );
               break;

          case STT_SECTION:
               waddstr ( menpad, "SECTION" );
               break;

          case STT_FILE:
               waddstr ( menpad, "FILE" );
               break;

          case STT_LOPROC:
               waddstr ( menpad, "LOPROC" );
               break;

          case STT_HIPROC:
               waddstr ( menpad, "HIPROC" );
               break;

          default:
               waddstr ( menpad, "(invalid)" );
               break;
     }

     /* "other field" */
     wmove ( menpad, 12, rescol );
     wprintw ( menpad, "0x%02x ", symbol->st_other );
     if ( !symbol->st_other )
          waddstr ( menpad, "(Normal)" );
     else
          waddstr ( menpad, "(Unexpected)" );

     /* Section name */
     /* We will only load the first 20 */
     if ( !symsecname )
          symsecname = "[NO ASSOICATION]";
          
     for ( i = 0; i < 20; i++ )
     {
          if ( !symsecname[i] )
               break;

          mvwaddch ( menpad, 13, (rescol + i), symsecname[i] );
     }
     
     /* Section index */
     wmove ( menpad, 14, rescol );
     wprintw ( menpad, "%i", symbol->st_shndx );
          
     switch ( UI_domenu ( menwin, menpad, 1 ) )
     {
          case 1:
               if ( sel_ref->adr != sel_adr )
                    dump_spawn ( );
               sel_finger = dump_mkref ( );
               sel_finger->ref = sel_adr;
               sel_finger->link = sel_ref;
               sel_finger->type = SEL_RELTARG;
               sel_finger->adr =
                    unitsheader->sh_offset +
                    rel->r_offset;
               sel_finger->size = 1;

               snprintf
               (
                    sel_finger->name,
                    SEL_NAMEMAX,
                    "@%s (%s)",
                    ( (unsigned char)rel->r_info > R_AVR_MAXTYPES ) ?
                    "???" :
                    reloc_avr[(unsigned char)rel->r_info],
                    (const char *)
                    &memmap[symstroff + symbol->st_name]
               );

               sel_ref = sel_finger;

               dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = sel_ref->adr;
               sel_size = sel_ref->size;
               dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               return;
               
               
          default:
               break;
     }

     delwin ( menwin );
     delwin ( menpad );
     delwin ( ifowin );

     return;
}


void dump_refop ( void )
{
     WINDOW *win, *diag;
     struct MenuItemList menu[6];

     int may, max, i;

     for ( i = 0; i < 6; i++ )
     {
          menu[i].item = sel_refop_list[i];
          menu[i].key  = sel_refop_keys[i];
     }

     win = UI_cwin
     (
          10, 28,
          "Reference Operation",
          "Select Operation"
     );

     getmaxyx ( win, may, max );
     may--, max--;

     switch ( UI_static_menu ( win, 2, menu, 6, 1 ) )
     {
          case 's':
               dump_spawn ( );
               break;
          case 'x':
          case 'X':
               dump_refexec ( );
               break;

          case 'o':
          case 'O':
               dump_selref ( );
               dump_highlight
               ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = sel_ref->adr;
               sel_size = sel_ref->size;
               dump_highlight
               ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          case 'r':
          case 'R':
               diag = UI_cwin
               (
                    5, 64,
                    "Rename Reference",
                    "Press Enter to Accept"
               );

               wattrset ( diag, A_REVERSE | A_UNDERLINE );
               for ( i = 0; i < 60; i++ )
                    mvwaddch ( diag, 2, (i+2), ' ' );
               UI_prompt ( diag, 2, 2, sel_ref->name, 60 );
               delwin ( diag );
               break;

          case 'd':
          case 'D':
               /* Delete reference */
               if ( (!sel_ref->prev) && (!sel_ref->next) )
               {
                    beep ( );
                    break;
               }

               free ( sel_ref->name );
               if ( (sel_ref->next) && (sel_ref->prev) )
               {
                    /* ha! */
                    sel_ref = sel_ref->next;
                    sel_ref->prev = sel_ref->prev->prev;
                    free ( sel_ref->prev->next );
                    sel_ref->prev->next = sel_ref;
               }
               else if ( sel_ref->next )
               {
                    sel_ref = sel_ref->next;
                    free ( sel_ref->prev );
                    sel_ref->prev = NULL;

                    /* Note, this must be the list head */
                    /* so update sel_list */
                    sel_list = sel_ref;
               }
               else
               {
                    sel_ref = sel_ref->prev;
                    free ( sel_ref->next );
                    sel_ref->next = NULL;
               }

               /* Reset selection */
               dump_highlight
               ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = sel_ref->adr;
               sel_size = sel_ref->size;
               dump_highlight
               ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;
               
          default:
               break;
     }

     delwin ( win );
     return;
}


void dump_selref ( void )
{
     WINDOW *win, *men, *pad;
     int i, j, may, max, sels, acti;

     struct Selection * sel_finger;

     getmaxyx ( actscr, may, max );
     may--, max--;

     sel_finger = sel_list;

     /* First we need to determine the number of   */
     /* selection items we have at this point, and */
     /* which one is active */
     sels = 1;
     acti = 1;
     while ( sel_finger->next )
     {
          sels++;
          sel_finger = sel_finger->next;
          
          if ( sel_finger == sel_ref )
               acti = sels;
     }

     /* Determine max window height */
     i = (may - (may / 4));

     if ( (sels + 5) < i )
          i = (sels + 5);

     win = UI_cwin
     (
          i, (max/2),
          "Select Reference",
          "Press ENTER to select"
     );

     touchwin ( win );
     wrefresh ( win );

     getmaxyx ( win, may, max );
     may--, max--;

     /* Set up actual menu area subwindow */
     men = derwin ( win, sels, (max - 4), 2, 2 );
     if ( !men )
          err_nomem ( );

     getmaxyx ( men, may, max );
     may--, max--;

     /* Set-up buffer based on menu area */
     pad = newpad ( sels, (max + 1) );
     if ( !pad )
          err_nomem ( );

     /* Clear the pad */
     getmaxyx ( pad, may, max );
     may--, max--;

     wattrset ( pad, A_REVERSE );
     for ( i = 0; i <= may; i++ )
          for ( j = 0; j <= max; j++ )
               mvwaddch ( pad, i, j, ' ' );

     sel_finger = sel_list;
     
     for ( i = 0; i < sels; i++ )
     {
          if ( sel_finger->name )
               mvwaddstr ( pad, i, 0, sel_finger->name );
          else
               mvwaddstr ( pad, i, 0, "[Default]" );

          sel_finger = sel_finger->next;
     }

     /* this is it! */
     acti = (UI_domenu ( men, pad, (acti - 1) ) + 1);

     delwin ( pad );
     delwin ( men );
     delwin ( win );

     if ( !acti )
          /* Canceled! */
          return;

     /* set-up new reference */
     sel_ref = sel_list;
     for ( i = 1; i < acti; i++ )
          sel_ref = sel_ref->next;

     /* Cool */
     return;
}

void dump_refexec ( void )
{
     struct Selection *popref;
     
     switch ( sel_ref->type )
     {
          case SEL_EHEADER:
               dump_info_eheader ( );
               break;

          case SEL_PHEADER:
               dump_info_pheader ( );
               break;

          case SEL_SHEADER:
               dump_info_sheader ( );
               break;

          case SEL_SYMBOL:
               dump_info_symtab ( );
               break;

          case SEL_REL:
          case SEL_RELA:
               dump_info_reloc ( );
               break;

          case SEL_RELTARG:
               popref = sel_ref;
               sel_ref = popref->link;
               
               dump_highlight
               ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = sel_ref->adr;
               sel_size = sel_ref->size;
               dump_highlight
               ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;

               dump_info_reloc ( );

               sel_ref = popref;
               dump_highlight
               ( UI_UNHIGHLIGHT, sel_adr, sel_size );
               sel_adr  = sel_ref->adr;
               sel_size = sel_ref->size;
               dump_highlight
               ( UI_HIGHLIGHT, sel_adr, sel_size );
               win_adr = sel_adr;
               break;

          default:
               break;
     }

     return;
}

void dump_spawn ( void )
{
     struct Selection * sel_finger;
     int key;

     /* First verify selection region */
     if ( sel_ref->size != sel_size )
     {
          key = UI_msgbox
                (
                     "Confirm Region",
                     "Selection does not match reference",
                     "Press Y to resize"
                );

          if ( (key != 'y') && (key != 'Y') )
          {
               UI_msgbox
               (
                    "Spawn Canceled",
                    "Spawn was canceled",
                    "Press any key to continue"
               );
               return;
          }

          if ( (sel_adr + sel_ref->size) > max_adr )
          {
               UI_msgbox
               (
                    "Spawn Failed",
                    "Cannot fit region",
                    "Press any key to continue"
               );
               return;
          }

     }

     /* Kill the highlight now */
     dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
     sel_size = sel_ref->size;
     
     /* Allocate a new reference entry */
     sel_finger = sel_list;
     while ( sel_finger->next )
          sel_finger = sel_finger->next;
              
     sel_finger->next =
          (struct Selection *)
          malloc ( sizeof ( struct Selection ) );
              
     if ( !sel_finger->next )
          err_nomem ( );

     /* Copy reference */
     memcpy
     (
          sel_finger->next,
          sel_ref,
          sizeof ( struct Selection )
     );

     sel_finger->next->link = sel_ref;
     sel_ref = sel_finger->next;

     /* Allocate new name string */
     sel_ref->name = malloc ( sizeof ( char ) * SEL_NAMEMAX );
     if ( !sel_ref->name )
          err_nomem ( );

     snprintf
     (
          sel_ref->name,
          sizeof ( char ) * SEL_NAMEMAX,
          "%s@",
          sel_ref->link->name
     );

     /* Set up the rest of the reference */
     sel_ref->prev = sel_finger;
     sel_ref->next = NULL;
     sel_ref->adr  = sel_adr;
     sel_ref->size = sel_size;
     
     /* Update selection */

     dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );

     return;
}

struct Selection * dump_mkref ( void )
{
     struct Selection * finger;
     unsigned int i;

     /* Seek to end of refrence list */
     finger = sel_list;

     while ( finger->next )
          finger = finger->next;

     /* Allocate memory for new selection */
     finger->next =
          (struct Selection *)
          malloc ( sizeof ( struct Selection ) );

     if ( !finger->next )
          err_nomem ( );

     finger->next->prev = finger;
     finger->next->next = NULL;
     finger = finger->next;

     /* Allocate space for name */
     finger->name = (char *)malloc ( SEL_NAMEMAX );

     if ( !finger->name )
          err_nomem ( );

     return finger;
}





