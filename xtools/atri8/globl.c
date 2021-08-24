/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */

#include <stdio.h>
#include <curses.h>
#include "globl.h"

WINDOW  *homescr,           // The main start-screen
        *dumpscr,           // screen for hex dump of the file itself
        *dumpwin,           // subwindow for containing the dump itself
        *infoscr,           // Main info screen
        *reloscr,           /* Relocation Analysis */
        *regscr,            /* Registration Packaging */
        *simscr,            /* ATRIA Simulation */
        *dumppad,           // For big data dumps!
        *actscr;            /* Active screen */

int     termx,              // terminal's max x
        termy,              // max y
        algn;               // global alignment setting

FILE    *elfmain,           // The primary elf file
        *outfile;           // For putting anything out

long    symstroff;         /* Symbol String Section Offset */

long    max_adr;           /* Max address of image */

long    sel_adr, win_adr;  /* Analysis Tool Selection Address */
int     sel_size;          /* Selection size */
struct Selection * sel_list, * sel_ref;

unsigned char isLittleEndian;

unsigned char * memmap;     /* Outgoing Memory map */


/*

    Program globals

*/


Elf32_Ehdr eheader;         // the actual ELF header as loaded




void err_nomem ( void )
{
    fclose ( elfmain );
    endwin ( );
    printf ( "Not enough memory. \n\n" );
    exit ( 1 );

    return;
}


struct Auto_Set Auto =
{
     .at     = 0x00,

     .img    = NULL,
     .pkg    = NULL,
     .cpoint = NULL,
     
     .cdd    = "main",
     .sid    = 0,
     .usec   = 0xffff,
     .dsec   = 0x0000,
     .bpri   = 0x0000,

     .tx     = 0x0000,
     .isr    = NULL,
     .irq    = 0x0000
};
