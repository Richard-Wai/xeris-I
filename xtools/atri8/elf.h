/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

#ifndef ELF_H
#define ELF_H

struct elf_sections
{
     char       * name;
     Elf32_Shdr * sheader;
     long         adr;
};

struct elf_symbols
{
     char       * name;
     Elf32_Sym  * sym;
};

/* Open object file */
extern void elf_open_object ( char * filename );

/* Seek to symbol string table */
extern long elf_seek_symstroff ( void );

/* Extract All Section Information */
extern int elf_get_sections ( struct elf_sections ** sections );
extern Elf32_Shdr * elf_seek_section
( const char * name, struct elf_sections * list, int total );

/* Get a list of all symbols */
extern int elf_get_symbols ( struct elf_symbols ** symbols );
extern Elf32_Sym * elf_seek_symbol
( const char * name, struct elf_symbols * list, int total );

#endif
