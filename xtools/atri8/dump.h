/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/*                                                              */
/*                 HEX Dump Screen and Functions                */
/*                                                              */

#ifndef DUMP_H
#define DUMP_H

#include "globl.h"

/* hex-dump screen */
extern long dump_init ( void );
extern void dump_base ( void );
extern void dump_kill ( void );

extern int  dump_commen ( void );
extern void dump_refop ( void );
extern void dump_refexec ( void );
extern void dump_spawn ( void );
extern struct Selection * dump_mkref ( void ); 

extern void dump_selref ( void );

extern void dump_refresh ( long nadr );
extern void dump_centre ( long * adr );
extern void dump_select ( long * dump_addr, long * sel_adr, int * size );

extern void dump_highlight ( unsigned char mode, long adr, int size );
extern long dump_limit ( long adr );

/* Parse the elf header and display the information in a window */
extern void dump_info_eheader ( void );
extern void dump_info_pheader ( void );
extern void dump_info_sheader ( void );
extern void dump_info_sheader_doref ( Elf32_Shdr *, unsigned char *, long );
extern void dump_info_symtab  ( void );
extern void UI_info_reloc   ( void );


#endif
