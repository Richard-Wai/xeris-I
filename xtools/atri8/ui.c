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




/*

    This function is for repeated code to control a menu
    Structure should be to provide a target window of the appropriate size
    as well as to provide a "Reference pad" of exactly the y
    length of menu items.

    The menu can be initialized to any item by setting stel.

    If any ket except arrow keys is pressed, the function returns
    whichever element was selected

*/

int UI_domenu ( WINDOW *tw, WINDOW *rp, int stel )
{
    int maxy, maxx, maxel, padw, i, j, offset, src;
    WINDOW *spad;   // for "selected" elements

    /* Disabled for auto */
    if (Auto.at)
         return ( 0 );

    // get length of window, and length of menu
    getmaxyx ( rp, maxel, padw );
    getmaxyx ( tw, maxy, maxx );
    maxel--, maxy--, maxx--, padw--;

    /* create pad for "selected" strings */
    if ( !( spad = dupwin ( rp ) ) )
        err_nomem ( );

    wattrset ( spad, A_NORMAL );

    // blank-out spad
    for ( i = 0; i <= maxel; i++ )
        for ( j = 0; j <= padw; j++ )
        {
            wmove ( spad, i, j);
            // pick up the character and strip the attributes
            // put it back down naked so that it takes on default attributes
            // (make everything A_REVERSE)

            waddch ( spad, (char)( winch( spad ) & A_CHARTEXT ) );
        }

    // how much scroll to apply at start
    offset = ( stel > maxy ) ? stel - maxy : 0;

    touchwin ( tw );

    while ( 1 )
    {
        for ( i = 0; ( i <= maxy ) && ( ( offset + i ) <= maxel ); i++ )
        {
            src = i + offset;

            if ( src == stel )
            {
                // load this line in from the spad!
                copywin ( spad, tw, src, 0, i, 0, i, maxx, FALSE );
                /* put the cursor at the end of the selection.. */
                wmove ( tw, i, maxx );
            }
            else
                copywin ( rp, tw, src, 0, i, 0, i, maxx, FALSE );
        }

        wrefresh ( tw );

        switch ( getch ( ) )
        {
            case KEY_UP:
                 stel--;

                 if ( (offset > 0) && (( stel - offset ) < 0) )
                      --offset;

                 if ( stel < 0 )
                 {
                      stel = maxel;
                      offset = (stel > maxy) ? (stel - maxy) : 0;
                 }

                 break;

            case KEY_DOWN:
                 stel++;

                 if ( ( stel - offset ) > maxy )
                     ++offset;

                 if ( stel > maxel )
                 {
                      stel = 0;
                      offset = 0;
                 }

                 break;

            case KEY_ENTER:
            case KEY_RETURN:
                return stel;

            case 'q':
            case 'Q':
                return ( -1 );

            default:
                break;

        }


    }

    delwin ( spad );

    return stel;
}

int UI_static_menu
( WINDOW * win, int align, struct MenuItemList * menu, int items, int start )
{
     int may, max, i, sel, ret, key, comp, topy, topx, mult;

     if (Auto.at)
          return ( 0 );

     sel = start;
     getmaxyx ( win, may, max );
     may--,max--;

     mult = (win == actscr) ? 3 : 1;

     /* First build the list */
     topy = (((may-2) / 2) - ((mult * items) / 2) + 1);

     
     topx = align;

     if ( mult > 1 )
          wattrset ( win, A_NORMAL );
     else
          wattrset ( win, A_REVERSE );
     
     for ( i = 0; i < items; i++ )
     {
          wmove ( win, (topy + (mult * i) + 1), (topx + 1) );
          wprintw ( win, "(%c)    %s", menu[i].key, menu[i].item );

     }

     /* Go into selection loop */
     ret = 0;
          
     do
     {
          if ( mult > 1 )
               wattrset ( win, A_REVERSE );
          else
               wattrset ( win, A_NORMAL );
          
          wmove ( win, (topy + (mult * (sel - 1)) + 1), (topx + 1) );
          wprintw ( win, "(%c)", menu[(sel - 1)].key );

          
          if ( mult > 1 )
               wattrset ( win, A_NORMAL );
          else
               wattrset ( win, A_REVERSE );
          
          waddstr ( win, "   " );

          wrefresh ( win );

          wmove ( win, (topy + (mult * (sel - 1)) + 1), (topx + 1) );
          wprintw ( win, "(%c)", menu[(sel - 1)].key );          

          
          flushinp ( );
          key = getch ( );

          switch ( key )
          {
               case KEY_UP:
                    if ( sel == 1 )
                         sel = items;
                    else
                         sel--;
                    break;

               case KEY_DOWN:
                    if ( sel == items )
                         sel = 1;
                    else
                         sel++;
                    break;

               case KEY_ENTER:
               case KEY_RETURN:
                    ret = menu[(sel - 1)].key;
                    break;

              default:
                   key = isalnum ( key ) ? toupper (key) : key;

                   for ( i = 0; i < items; i++ )
                   {
                        comp = isalnum ( menu[i].key ) ?
                               toupper ( menu[i].key ) :
                               menu[i].key;

                        if ( key == comp )
                        {
                             ret = menu[i].key;
                             break;
                        }
                   }
                   break;
          }
          
     }while ( !ret );
                    
     return ( ret );
}

     
int UI_table_menu
( WINDOW *tscr, WINDOW *srcpad, int srow, int erow, int scol, int start )
{
     WINDOW * render;
     int may, max, smay, smax, pay, pax, ray, rax, isel,
         imax, page, lastpage, pagesize, i, c;

     if (Auto.at)
          return ( 0 );
     
     page   = 0;

     pagesize = erow - srow;

     getmaxyx ( tscr, smay, max );
     smay--, smax--;

     getmaxyx ( srcpad, pay, pax );
     pay--, pax--;

     render = derwin ( tscr, pagesize, (pax + 1), srow, scol );

     if ( !render )
          err_nomem ( );

     wattrset ( render, A_NORMAL );

     getmaxyx ( render, ray, rax );
     ray--, rax--;

     /* Ok, seek page-number and get into the main loop */

     page = start / pagesize;
     isel = start - (pagesize * page);
     imax = ((pagesize * (page + 1)) > pay) ?
          (pay - (pagesize * page)) : ray;

     if ( isel > imax )
          isel = imax;
     
     lastpage = page + 1;

     while ( 1 )
     {
          if ( page != lastpage )
          {

               /* Refresh the window */
               wclear ( render );
               copywin
               (
                    srcpad, render,
                    (pagesize * page), 0,
                    0, 0,
                    (((pagesize * page) + ray) > pay) ?
                    (pay - (pagesize * page)) :
                    ray,
                    pax,
                    FALSE
               );

               imax = ((pagesize * (page + 1)) > pay) ?
                    (pay - (pagesize * page)) : ray;
               
               if ( isel > imax )
                    isel = imax;
          }

          lastpage = page;

          /* Ok, so, now we can do the selection */
          wattrset ( render, A_REVERSE );

          for ( i = 0; i <= rax; i++ )
          {
               wmove ( render, isel, i );
               waddch ( render, (char)( winch( render ) & A_CHARTEXT ) );
          }
          wmove ( render, isel, rax );

          wattrset ( render, A_NORMAL );

          
          wrefresh ( render );
          flushinp ( );
          c = getch ( );

          /* Remove selection */
          for ( i = 0; i <= rax; i++ )
          {
               wmove ( render, isel, i );
               waddch ( render, (char)( winch( render ) & A_CHARTEXT ) );
          }

          switch ( c )
          {
               case KEY_UP:
                    if ( isel == 0 )
                    {
                         if ( page == 0 )
                         {
                              beep ( );
                              break;
                         }

                         page--;
                         isel = pagesize;
                         break;
                    }

                    isel--;
                    break;

               case KEY_DOWN:
                    if ( isel == imax )
                    {
                         if ( (pagesize * (page + 1)) > pay )
                         {
                              beep ( );
                              break;
                         }

                         page++;
                         isel = 0;
                         break;
                    }

                    isel++;
                    break;

               case KEY_PPAGE:
                    if ( page == 0 )
                    {
                         beep ( );
                         break;
                    }

                    page--;
                    break;

               case KEY_NPAGE:
                    if ( (pagesize * (page + 1)) > pay )
                    {
                         beep ( );
                         break;
                    }

                    page++;
                    if ( isel > imax )
                         isel = imax;
                    break;

               case KEY_ENTER:
               case KEY_RETURN:
                    delwin ( render );
                    return ( (pagesize * page) + isel );

               default:
                    delwin ( render );
                    return ( -1 * c );
          }
     }


}


void UI_cputs ( WINDOW *twin, int trow, char * tstr )
{
    int len, indent, y, width;

    if (Auto.at)
         return;

    getmaxyx ( twin, y, width );        // get screen width

    len = strlen ( tstr );              // get target string's length
    indent = width - len;               // subtract it from screen width
    indent /= 2;                        // divide result by two

    mvwaddstr ( twin, trow, indent, tstr );

    return;
}

void UI_fwin ( WINDOW *twin, char *title, char *bar )
{
    int max, may, i;

    if (Auto.at)
         return;

    getmaxyx ( twin, may, max );
    may--, max--;

    wattrset ( twin, A_REVERSE );

    for ( i = 0; i <= max; i++ )
    {
        mvwaddch ( twin, 0, i, ' ' );
        mvwaddch ( twin, may, i, ' ' );
    }

    UI_cputs ( twin, 0, title );
    mvwaddstr ( twin, may, 1, bar );
    wmove ( twin, may, max );

    wattrset ( twin, A_NORMAL );

    return;

}

WINDOW *UI_cwin ( int rows, int cols, char *title, char *ifobar )
{
    WINDOW *wp;
    int down, over;
    register int i, j;
    int smay, smax, may, max;

    if (Auto.at)
         return (NULL);

    getmaxyx ( actscr, smay, smax );
    smay--, smax--;
    
    down = ( smay - rows ) / 2;
    over = ( smax - cols ) / 2;
    ++down; ++over;

    if ( !(wp = newwin ( rows, cols, down, over )) )
         err_nomem ( );

    getmaxyx ( wp, may, max );

    /* Clear content */

    wattrset ( wp, A_REVERSE );
    for ( i = 0; i <= may; i++ )
         for ( j = 0; j <= max; j++ )
         mvwaddch ( wp, i, j, ' ' );



    box ( wp, 0, 0 );
    UI_cputs ( wp, 0, title );
    UI_cputs ( wp, ( rows - 1 ), ifobar );

    return ( wp );
}


void UI_prompt ( WINDOW * win, int brow, int bcol, char * data, int len )
{
     /* Handle an old-school prompt, by hand */
     int pos, lim, may, max, bin;

     if (Auto.at)
          return;
     
     pos = 0, lim = 0;
     getmaxyx ( win, may, max );
     may--, max--;

     lim = max - bcol;

     pos = strnlen ( data, lim );

     mvwaddstr ( win, brow, bcol, data );
     

     while ( 1 )
     {
          wrefresh ( win );
          flushinp ( );

          switch ( bin = getch () )
          {
               case KEY_ENTER:
               case KEY_RETURN:
                    data[pos] = '\0';
                    return;

               case KEY_BACKSPACE:
               case 0x7f:
               case 0x08:
                    if ( pos == 0 )
                    {
                         beep ( );
                         break;
                    }

                    data[--pos] = '\0';
                   
                    mvwaddch ( win, brow, (bcol + pos), ' ' );
                    wmove ( win, brow, (bcol + pos) );                   
                   
                    break;

              default:
                   if ( !isprint ( bin ) )
                   {
                        beep ( );
                        data[pos] = '\0';
                        break;
                   }

                   if ( (pos >= len) || (pos > lim) )
                   {
                        beep ( );
                        data[pos] = '\0';
                        break;
                   }
                 
                   data[pos] = (char)bin;
                   waddch ( win, data[pos] );
                   pos++;
                   



                   break;
          }



     }
}
          
int UI_msgbox ( char * title, char * msg, char * footer )
{
     int smay, smax;
     register int key;
     WINDOW * dialog;

     if (Auto.at)
     {
          printf ( "%s\n", msg );
          return ( 0 );
     }

     getmaxyx ( actscr, smay, smax );
     
     dialog = UI_cwin
     (
          7, (smax / 2),
          title,
          footer
     );

     mvwaddstr ( dialog, 3, 2, msg );
     wrefresh ( dialog );

     key = getch ( );
     
     delwin ( dialog );
     touchwin ( actscr );
     wrefresh ( actscr );

     return ( key );

}



