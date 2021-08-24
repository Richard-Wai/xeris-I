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

void elf_open_object ( char *filename )
{
     WINDOW * dialog;
     int smay, smax, dmay, dmax;

     register int i;

     getmaxyx ( actscr, smay, smax );

 retry:
     
     if ( !filename[0] )
     {
          /* Ask for a new file */
          dialog = UI_cwin
          (
               10, (smax / 2),
               "Load Target Object File",
               "Press Enter When Finished"
          );

          getmaxyx ( dialog, dmay, dmax );
          mvwaddstr ( dialog, ((dmay/2) - 1), 2,
                      "Enter File Name:" );
          
          wattrset ( dialog, A_REVERSE | A_UNDERLINE );
          
          for ( i = 2; i < (dmax - 2); i++ )
               mvwaddch ( dialog, (dmay/2), i, ' ' );
          
          wrefresh ( dialog );
               
          UI_prompt ( dialog, (dmay/2), 2, filename,
                      ((dmax - 3) < 1023) ? ((dmax - 3) - 3) : 1023 );

          delwin ( dialog );
          touchwin ( actscr );
          wrefresh ( actscr );
     }

     elfmain = fopen ( filename, "r+" );


          

     if ( !elfmain )
     {

          if (Auto.at)
               /* Don't retry, just blank it */
               return;
          
          switch
          (
               UI_msgbox
               (
                    " ERROR ! ",
                    "Unable to open file",
                    "Press a Key to Retry, or 'c' to Cancel"
               )
          )
          {
               case 'c':
               case 'C':
                    delwin ( dialog );
                    touchwin ( actscr );
                    wrefresh ( actscr );
                    filename[0] = '\0';
                    elfmain = NULL;
                    return;

               default:
                    break;
          }
               

          filename[0] = '\0';
          goto retry;
     }

     

    
     return;
}


long elf_seek_symstroff ( void )
{
     long check_adr, shstroff;
     Elf32_Shdr *sheader, *strtab;

     symstroff = 0;

     if ( eheader.e_shentsize != sizeof ( Elf32_Shdr ) )
     {
          UI_msgbox
          (
               "INVALID",
               "Bad Symbol String Table!",
               "Press any key to continue"
          );
          symstroff = 1;
          return 1;
     }

     /* Load section string table */
     check_adr =
          eheader.e_shoff +
          (eheader.e_shstrndx * eheader.e_shentsize);

     if ( check_adr > max_adr )
     {
          UI_msgbox
          (
               "ERROR",
               "ELF Header: Bad Section String Table",
               "Pree any key to continue"
          );
          symstroff = 1;
          return 1;
     }

     strtab = (Elf32_Shdr *)&memmap[check_adr];
     shstroff = strtab->sh_offset;
     
     check_adr = eheader.e_shoff;
     /* Iterate until we have a match */
     while ( !symstroff )
     {
          if ( (check_adr + sizeof ( Elf32_Shdr )) > max_adr )
          {
               UI_msgbox
               (
                    "INVALID",
                    "No symbol string table, or bad ELF",
                    "Pree any key to cancel"
               );
               symstroff = 1;
               return 1;
          }

          sheader = (Elf32_Shdr *)&memmap[check_adr];
          
          /* Is it a string table? */
          if ( sheader->sh_type == SHT_STRTAB )
          {
               if
               (
                    !strcmp
                    (
                         (const unsigned char *)
                         &memmap[shstroff + sheader->sh_name],
                         ".strtab"
                    )
               )
                    /* contact! */
                    symstroff = sheader->sh_offset;
          }

          check_adr += eheader.e_shentsize;
     }

     return ( symstroff );
}


int elf_get_sections ( struct elf_sections ** sections )
{
     Elf32_Shdr *sheader;
     char *strtab;

     struct elf_sections *seclist;
     
     register int i;
     /* First lets find out how many sections we're */
     /* dealing with, and allocate space for those */
     seclist = malloc
          ( sizeof ( struct elf_sections ) * eheader.e_shnum );

     *sections = seclist;

     if ( !sections )
          err_nomem ( );

     /* Load in section string header */
     sheader = (Elf32_Shdr *)&memmap
          [eheader.e_shoff + (eheader.e_shstrndx * eheader.e_shentsize)];
     strtab = (char *)&memmap[sheader->sh_offset];


     /* Ok, load'em in */
     for ( i = 0; i < (int)eheader.e_shnum; i++ )
     {
          seclist[i].adr =
               eheader.e_shoff + (eheader.e_shentsize * i);
          sheader = (Elf32_Shdr *)&memmap[seclist[i].adr];
          seclist[i].name = (char *)(strtab + sheader->sh_name);
          seclist[i].sheader = sheader;
     }

     return ( i );
}


extern Elf32_Shdr * elf_seek_section
( const char * name, struct elf_sections * list, int total )
{
     register int i;
     register size_t len;

     len = strnlen ( name, 256 );

     for ( i = 0; i < total; i++ )
          if ( !strncmp ( name, list[i].name, len ) )
               return ( list[i].sheader );

     return ( NULL );
}



extern int elf_get_symbols ( struct elf_symbols ** symbols )
{
     Elf32_Shdr * sheader, * finger;
     Elf32_Sym * symtab;
     char *shstrtab, *strtab;

     struct elf_symbols * symlist;

     register int i, symcount;

     /* Load in section string header */
     sheader = (Elf32_Shdr *)&memmap
          [eheader.e_shoff + (eheader.e_shstrndx * eheader.e_shentsize)];
     shstrtab = (char *)&memmap[sheader->sh_offset];

     /* Now we iterate over the sections until we find */
     /* the symtab */

     sheader = NULL;
     strtab  = NULL;
     finger  = (Elf32_Shdr *)&memmap[eheader.e_shoff];

     /* Ok, load'em in */
     for ( i = 0; i < (int)eheader.e_shnum; i++ )
     {
          if ( finger[i].sh_type == SHT_SYMTAB )
               sheader = &finger[i];
          else if ( strncmp ( &shstrtab[finger[i].sh_name],
                              ".strtab",
                              8 ) == 0 )
               strtab = (char *)&memmap[finger[i].sh_offset];

          if ( sheader && strtab )
               break;

     }

     if ( !sheader || !strtab )
          /* Didn't find them */
          return ( 0 );

     /* Set up the symtab pointer */
     symtab = (Elf32_Sym *)&memmap[sheader->sh_offset];

     /* Ok, now we are ready to rock it. First       */
     /* we need to calculate how many symbols are in */
     /* the table, then allocate the array */
     symcount = (int)sheader->sh_size / (int)sheader->sh_entsize;

     /* Allocate the array */
     symlist = malloc
          ( sizeof ( struct elf_symbols ) * symcount );

     *symbols = symlist;

     if ( !symlist )
          err_nomem ( );

     /* Finally, we load 'em all in! */
     for ( i = 0; i < symcount; i++ )
     {
          symlist[i].name = &strtab[symtab[i].st_name];
          symlist[i].sym  = &symtab[i];
     }

     return ( symcount );
}

extern Elf32_Sym * elf_seek_symbol
( const char * name, struct elf_symbols * list, int total )
{
     register int i;
     register size_t len;

     for ( i = 0; i < total; i++ )
          if ( !strncmp ( name, list[i].name, 256 ) )
               return ( list[i].sym );

     return ( NULL );
}
          
               
     
