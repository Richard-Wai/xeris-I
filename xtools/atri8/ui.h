/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

#ifndef UI_H
#define UI_H

#include <curses.h>
#include "globl.h"

extern void UI_inithome ( void );
extern void UI_homebase ( char * openfile );


/* Manage menu highlighting */
extern int UI_domenu ( WINDOW *tw, WINDOW *rp, int stel );
extern int UI_static_menu
( WINDOW * win, int align, struct MenuItemList * menu,
  int items, int start  );
extern int UI_table_menu
( WINDOW *tscr, WINDOW *srcpad, int srow, int erow, int scol, int start );

/* centre text */
extern void UI_cputs ( WINDOW *twin, int trow, char * tstr );

/* Set up fullscreen window top and bottom */
extern void UI_fwin ( WINDOW *twin, char *title, char *bar );

/* create a new window and centre it in the terminal */
extern WINDOW *UI_cwin ( int rows, int cols, char *title, char *ifobar );
extern void UI_prompt ( WINDOW *win, int brow, int bcol, char *data, int len );
extern int  UI_msgbox ( char * title, char * msg, char * footer );


#endif
