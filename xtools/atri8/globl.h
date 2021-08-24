/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

#ifndef GLOBL_H
#define GLOBL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <curses.h>

#include "elfdef.h"
#include "statdat.h"

/*

    Configuration values

*/


#define KEY_RETURN 0xa
#define KEY_TAB    0x9


#define UI_EHEADER_IFOWIN_ROWS  18
#define UI_EHEADER_IFOWIN_COLS  42

#define UI_HIGHLIGHT   0x00
#define UI_UNHIGHLIGHT 0x01

#define SEL_DEFAULT    0x00   /* Unspecified selection */
#define SEL_EHEADER    0x01   /* ELF Header            */
#define SEL_PHEADER    0x02   /* Program Header        */
#define SEL_SHEADER    0x03   /* Section Header        */
#define SEL_SYMBOL     0x04   /* Symbol table entry    */
#define SEL_STRING     0x05   /* String table entry    */
#define SEL_REL        0x06   /* Normal Relocation     */
#define SEL_RELA       0x07   /* Augmented Relocation  */
#define SEL_RELTARG    0x08   /* Relocation Target     */
#define SEL_RELATARG   0x09
#define SEL_OFFSET     0x0a   /* File offset           */
#define SEL_INDEX      0x0b
#define SEL_POINTER    0x0c   /* Memory map pointer    */
#define SEL_MECH       0x0d   /* Machine Code          */

#define SEL_NAMEMAX    60     /* Max name length       */

/* Structs */
struct MenuItemList
{
     const char * item;
     int          key;
};

/* Selection Object */
struct Selection
{
     struct Selection * prev;
     struct Selection * next;
     struct Selection * link;

     long               adr;
     int                size;

     char             * name;

     long               ref;      /* Specific Reference */

     unsigned char      type;
     
};


/* Automatic mode option set */
struct Auto_Set
{
     unsigned char      at;     /* 0x00 if inactive 0xff if active */
     

     char             * img;    /* Image file path                 */
     char             * pkg;    /* Package output path             */
     char             * cpoint; /* Connection point                */
     
     char             * cdd;    /* CDD Symbol name                 */
     uint16_t           sid;
     uint16_t           usec;
     uint16_t           dsec;
     uint16_t           bpri;

     uint16_t           tx;     /* TX account offset */
     char             * isr;    /* ISR symbol name   */
     uint16_t           irq;    /* IRQ number        */
};



/* symtab Info / Relocation Definition */
#define NOREL          0
#define REL            1
#define RELA           2
     
/* Globals */
extern WINDOW
     *homescr,
     *dumpscr,
     *dumpwin,
     *infoscr,
     *reloscr,
     *regscr,
     *simscr,
     *dumppad,
     *actscr;

extern int
     termx,
     termy,
     algn;

extern FILE
     *elfmain,
     *outfile;

extern long
     symstroff,
     max_adr,
     sel_adr,
     win_adr;

extern int sel_size;

extern struct Selection
     *sel_list,
     *sel_ref;

extern unsigned char
     isLittleEndian,
     *memmap;

extern Elf32_Ehdr
     eheader;

extern struct Auto_Set Auto;

extern void  err_nomem           ( void );
      
#endif
