/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curses.h>

#include "globl.h"
#include "elfdef.h"
#include "elf.h"
#include "ui.h"
#include "dump.h"
#include "relo.h"


static void calc_sum ( long sadr );
static  int show_sum ( int  );
static int show_detail ( unsigned char );
static WINDOW * calc_detail ( unsigned char ); 
static void set_filter ( void );
static void filter_relsec ( void );
static void sel_sym ( int, unsigned char );




/* UI-Specific stuff */
#define RELOC_COUNT_COL 20
#define RELOC_COUNT_LEN 8
#define RELOC_COUNT_FMT "%8i"

#define DETAIL_TARG_SEC 0
#define DETAIL_TARG_OFF 15
#define DETAIL_SYM_NAM  25
#define DETAIL_SYM_TYP  50
#define DETAIL_SYM_SEC  60


static WINDOW *
    *sumwin,                    /* Summary window */
    *sumpad,
    *lswin,                     /* List window    */
    *lspad;

#define MAINMENMAX


static unsigned int relcounts[R_AVR_MAXTYPES+2];
static struct elf_sections secnone, *seclist, **relseclist, *secfilter;
static int seccount, relseccount, filtermem, seltype;

void relo_init ( void )
{
     sumpad = NULL;
     lswin  = NULL;
     lspad  = NULL;

     seltype = 0;

     secnone.name    = "[ALL]";
     secnone.sheader = NULL;
     secnone.adr     = 0;
     seclist         = NULL;
     relseclist      = NULL;
     secfilter       = &secnone;
     seccount        = 0;
     relseccount     = 0;

     return;
}

void relo_reset ( void )
{
     if ( seclist )
          free ( seclist );

     if ( relseclist )
          free ( relseclist );

     relo_init ( );
     return;
}



void relo_base ( void )
{
     int i, smay, smax, key, mark;

     getmaxyx ( reloscr, smay, smax );
     smay--, smax--;
     
     actscr = reloscr;

     mark = 0;

     while ( 1 )
     {
          wattrset ( actscr, A_REVERSE );
          for ( i = 0; i < smax; i++ )
               mvwaddch ( actscr, smay, i, ' ' );

          wmove ( actscr, smay, 1 );
          wprintw
          (
               actscr,
               "Filtering Section: %s",
               secfilter->name
          );

          mvwaddstr ( actscr, smay, smax - 16,  "(f) Set Filter." );
          wattrset ( actscr, A_NORMAL );
          touchwin ( actscr );
          wrefresh ( actscr );

          key = show_sum ( mark );

          if ( key < 0 )
               key *= -1;
          else
               show_detail ( key );

          switch ( key )
          {
               case 'q':
               case 'Q':
                    return;

               case 'f':
               case 'F':
                    set_filter ( );
                    delwin ( sumpad );
                    sumpad = NULL;
                    break;

               default:
                    break;

          }
     }
     
}


static void calc_sum ( long sadr )
{
     Elf32_Shdr *sheader;
     Elf32_Rel *rel; 
     int smay, smax, i, ents, onepass;
     long radr;
     unsigned char reltype;

     getmaxyx ( reloscr, smay, smax );
     smay--, smax--;
     
     if ( sumpad )
          delwin ( sumpad );

     sumpad = NULL;

     if ( (RELOC_COUNT_COL + RELOC_COUNT_LEN) > (smax - 4) )
     {
          UI_msgbox
          (
               "ERROR !",
               "Your terminal is too narrow.",
               "Press any key to escape"
          );

          return;
     }
     

     sumpad = newpad ( R_AVR_MAXTYPES + 2,
                       RELOC_COUNT_COL + RELOC_COUNT_LEN + 1 );
     

     if ( !sumpad )
          err_nomem ( );

     /* Ok so the pad is good, now we need to go through */
     /* each relocation entry in the provided section,  */
     /* and list out their types */

     wattrset ( sumpad, A_NORMAL );

     for ( i = 0; i <= R_AVR_MAXTYPES; i++ )
     {
          mvwaddstr ( sumpad, i, 0, reloc_avr[i] );
          relcounts[i] = 0;
     }

     mvwaddstr ( sumpad, i, 0, "*UNKNOWN*" );
     relcounts[i] = 0;

     /* We will scan through every relocation section, and */
     /* every relocation entry                             */

     if ( !sadr )
     {
          onepass = 0;
          sadr = eheader.e_shoff;
     }
     else
     {
          onepass = 1;
     }

     ents = 1;
     sheader = (Elf32_Shdr *)&memmap[sadr];
     rel = NULL;
     
     
     do
     {
          if
          (
               (sheader->sh_type == SHT_REL ) ||
               (sheader->sh_type == SHT_RELA)
          )
          {
               /* We got a section header, lets start loading */
               /* it up */
               radr = (long)sheader->sh_offset;
               rel = (Elf32_Rel *)&memmap[radr];
               i = 1;

               while ( 1 )
               {
                    reltype = (unsigned char)rel->r_info;
                    
                    if ( reltype > R_AVR_MAXTYPES )
                         reltype = (R_AVR_MAXTYPES + 1);

                    relcounts[reltype]++;
                    i++;

                    if ( (i * sheader->sh_entsize) > sheader->sh_size )
                         break;

                    radr += sheader->sh_entsize;
                    rel = (Elf32_Rel *)&memmap[radr];
               }
          }

          ents++;
               
          if ( ents > eheader.e_shnum )
               /* End of the sections */
               break;

               
          sadr += (long)eheader.e_shentsize;
          sheader = (Elf32_Shdr *)&memmap[sadr];
          
     }while ( !onepass );


     /* Cool, lets print-out our tabulations */
     for ( i = 0; i <= (R_AVR_MAXTYPES+1); i++ )
     {
          wmove ( sumpad, i, RELOC_COUNT_COL );
          wprintw ( sumpad, RELOC_COUNT_FMT, relcounts[i] );
     }

     /* We're done here */
     return;
}


static int show_sum ( int start )
{
     WINDOW *oldscr;
     int may, max, res;

     getmaxyx ( reloscr, may, max );
     may--, max--;

     oldscr = dupwin ( reloscr );
     if ( !oldscr )
          err_nomem ( );

     if ( !sumpad )
          calc_sum ( secfilter->adr );

     wattrset ( reloscr, A_BOLD | A_UNDERLINE );
     mvwaddstr ( reloscr, 2, 2, "Relocation Type" );
     mvwaddstr ( reloscr, 2, 2 + RELOC_COUNT_COL + 1, "Entries" );
     wattrset ( reloscr, A_NORMAL );
     wrefresh ( reloscr );

     res = UI_table_menu
           (
                reloscr,
                sumpad,
                3,
                may - 2,
                2,
                start
           );

     /* Restore screen */
     overwrite ( oldscr, reloscr );
     delwin ( oldscr );

     wrefresh ( reloscr );
     return ( res );

}



static void set_filter ( void )
{
     int smay, smax, thisy, thisx, pay, pax, i, j, newsel;
     WINDOW *menwin, *menarea, *menpad;

     struct elf_sections *relsec;
     
     if ( !seclist )
          /* populate section listing */
          seccount     = elf_get_sections ( &seclist );

     if ( !relseclist )
     {
          filter_relsec ( );
          filtermem = 0;
     }


     /* First thing, we need to filter our list */

     getmaxyx ( actscr, smay, smax );
     smay--, smax--;


     if ( !menpad )
          err_nomem ( );


     /* Create a window */
     menwin = UI_cwin
              (
                   smay - 6, 40,
                   "Set Section Filter",
                   "Press ENTER to Select, \'q\' to Cancel"
              );

     touchwin ( menwin );
     wrefresh ( menwin );

     getmaxyx ( menwin, thisy, thisx );
     thisy--, thisx--;
     
     menarea = derwin ( menwin, (thisy - 3), (thisx - 4), 2, 2 );

     if ( !menarea )
          err_nomem ( );
     
     /* Build a menu board */
     menpad = newpad ( relseccount, thisx + 1 );

     if ( !menpad )
          err_nomem ( );


     /* Clear the menu pad */
     getmaxyx ( menpad, pay, pax );
     pay--, pax--;

     wattrset ( menpad, A_REVERSE );
     for ( i = 0; i <= pay; i++ )
          for ( j = 0; j <= pax; j++ )
               mvwaddch ( menpad, i, j, ' ' );
     
     /* Load menu pad */
     for ( i = 0; i < seccount; i++ )
          mvwaddstr
          (
               menpad, i, 0,
               relseclist[i]->name
          );


     /* Do the menu! */
     newsel = UI_domenu ( menarea, menpad, filtermem );

     if ( newsel >= 0 )
     {
          secfilter = relseclist[newsel];
          filtermem = newsel;
     }

     delwin ( menwin );
     delwin ( menarea );
     delwin ( menpad );
     return;
}
     


static void filter_relsec ( void )
{
     register int i, j;

     if ( relseclist )
          free ( relseclist );
     
     /* Stage one, find the count */
     relseccount = 0;

     for ( i = 0; i < seccount; i++ )
          if
          (
               (seclist[i].sheader->sh_type == SHT_REL ) ||
               (seclist[i].sheader->sh_type == SHT_RELA)
          )
               relseccount++;

     /* Add one for "all" */
     relseccount++;

     /* Now make space for the pointers */
     relseclist =
          malloc ( sizeof ( struct elf_sections * ) * relseccount );

     if ( !relseclist )
          err_nomem ( );

     /* Inset the "ALL" one first */
     relseclist[0] = &secnone;
     
     /* Stage two */
     /* Now make the connections */
     
     for ( i = 0, j = 1; i < seccount; i++ )
          if
          (
               (seclist[i].sheader->sh_type == SHT_REL ) ||
               (seclist[i].sheader->sh_type == SHT_RELA)
          )
               relseclist[j++] = &seclist[i];

     return;
}


               
static int show_detail ( unsigned char rtype )
{
     WINDOW *detpad, *oldscr;
     int may, max, res;

     getmaxyx ( reloscr, may, max );

     oldscr = dupwin ( reloscr );
     if ( !oldscr )
          err_nomem ( );

     /* Make sure we have a section list populated */
     if ( !seclist )
          /* populate section listing */
          seccount     = elf_get_sections ( &seclist );

     if ( !relseclist )
     {
          filter_relsec ( );
          filtermem = 0;
          secfilter = &secnone;
     }

     wattrset ( reloscr, A_BOLD | A_UNDERLINE );
     mvwaddstr ( reloscr, 2, 2, "In Section" );
     mvwaddstr ( reloscr, 2, 2 + DETAIL_TARG_OFF, "Offset" );
     mvwaddstr ( reloscr, 2, 2 + DETAIL_SYM_NAM,  "Symbol" );
     mvwaddstr ( reloscr, 2, 2 + DETAIL_SYM_TYP,  "Type" );
     mvwaddstr ( reloscr, 2, 2 + DETAIL_SYM_SEC,  "Section" );
     wattrset ( reloscr, A_NORMAL );
     touchwin ( reloscr );
     wrefresh ( reloscr );
     
     /* Do it! */
     detpad = calc_detail ( rtype );


     /* Ok, let's set up  */
     res = UI_table_menu
           (
                reloscr,
                detpad,
                3,
                may - 2,
                2,
                0
           );

     if ( res >= 0 )
          sel_sym ( res, rtype );

     /* Restore screen */
     overwrite ( oldscr, reloscr );

     delwin ( oldscr );
     delwin ( detpad );
     touchwin ( reloscr );
     wrefresh ( reloscr );

     return ( res );

}


static WINDOW * calc_detail ( unsigned char rtype )
{
     WINDOW * detpad;

     Elf32_Shdr *relsheader, *symsheader, *targsheader, *symsecsheader;
     Elf32_Rel *thisrel;
     Elf32_Sym *thissym;
     char *symstrtab, *secstrtab, *targsec, *symsec, *symname;
     struct elf_sections **sections_base, *sections_dummy[2];

     int i, k, lim, line;

     detpad = newpad
          ( relcounts[rtype] ? relcounts[rtype] : 1,
            DETAIL_SYM_SEC + 15 );

     if ( !detpad )
          err_nomem ( );

     line = 0;

     /* Set up the "infrastructure" by seeking the section and */
     /* symbol string tables */
     symsheader = (Elf32_Shdr *)&memmap
          [eheader.e_shoff + (eheader.e_shentsize * eheader.e_shstrndx)];

     secstrtab = (char *)&memmap[symsheader->sh_offset];
     
     symstrtab = NULL;
     for ( i = 0; i < (int)eheader.e_shnum; i++ )
     {
          symsheader = (Elf32_Shdr *)&memmap
               [eheader.e_shoff + (eheader.e_shentsize * i)];

          if
          (
               !strncmp
               (
                    ".strtab",
                    &secstrtab[symsheader->sh_name],
                    10
               )
          )
          {
               /* Contact */
               symstrtab = (char *)&memmap[symsheader->sh_offset];
               break;
          }
     }


     sections_dummy[0] = NULL;
     sections_dummy[1] = secfilter;
     
     /* Set up the starting section at the filter */
     if ( (secfilter) && (secfilter != &secnone) )
          sections_base = sections_dummy;
     else
          sections_base = relseclist;

     
     /* Start cycling through each section in turn */
     for ( k = 1; k < relseccount; k++ )
     {

          relsheader = sections_base[k]->sheader;
          /* Load the section affected section name */
          targsheader = (Elf32_Shdr *)&memmap
               [eheader.e_shoff +
                (eheader.e_shentsize * relsheader->sh_info)];
          targsec = &secstrtab[targsheader->sh_name];

          
          /* Load the symbol table for this section */
          symsheader = (Elf32_Shdr *)&memmap
               [eheader.e_shoff +
                (eheader.e_shentsize * relsheader->sh_link)];

          /* How many relocations do we need to deal */
          /* with in this section? */
          lim = (int)relsheader->sh_size / (int)relsheader->sh_entsize;


          /* First lets grab each relocation entry */
          for ( i = 0; i < lim; i++ )
          {
               /* Point at the entry */
               thisrel = (Elf32_Rel *)&memmap
                    [relsheader->sh_offset +
                     (relsheader->sh_entsize * i)];

               /* Is it the right type? */
               if ( (unsigned char)thisrel->r_info != rtype )
                    continue;
               
               /* print out the target section */
               mvwaddstr ( detpad, line, DETAIL_TARG_SEC, targsec );

               /* Print-out the target offset */
               wmove ( detpad, line, DETAIL_TARG_OFF );
               wprintw ( detpad, "%08x", thisrel->r_offset );

               if ( (thisrel->r_info >> 8) == 0 )
               {
                    /* No associated symbol */
                    mvwaddstr ( detpad, line++, DETAIL_SYM_NAM,
                                "[NO SYMBOL]" );
                    continue;
               }
               
               /* Load the related symbol entry */
               thissym = (Elf32_Sym *)&memmap
                    [symsheader->sh_offset +
                     (symsheader->sh_entsize *
                      (thisrel->r_info >> 8) )];

               /* Print the symbol name */
               if ( thissym->st_name )
                    symname = &symstrtab[thissym->st_name];
               else
                    symname = "------";

               mvwaddstr ( detpad, line, DETAIL_SYM_NAM, symname );

               /* Process the symbol type */
               wmove ( detpad, line, DETAIL_SYM_TYP );
               switch ( thissym->st_info & 0x0f )
               {
                    case STT_NOTYPE:
                         waddstr ( detpad, "NO_TYPE" );
                         break;

                    case STT_OBJECT:
                         waddstr ( detpad, "OBJECT" );
                         break;

                    case STT_FUNC:
                         waddstr ( detpad, "FUNC" );
                         break;

                    case STT_SECTION:
                         waddstr ( detpad, "SECTION" );
                         break;

                    case STT_FILE:
                         waddstr ( detpad, "FILE" );
                         break;

                    case STT_LOPROC:
                         waddstr ( detpad, "LOPROC" );
                         break;

                    case STT_HIPROC:
                         waddstr ( detpad, "HIPROC" );
                         break;

                    default:
                         waddstr ( detpad, "*UNKWN*" );
                         break;
               }

               /* Finally print the symbol's section */
               if ( thissym->st_shndx >= SHN_LORESERVE )
               {
                    /* No associated section */
                    mvwaddstr ( detpad, line++, DETAIL_SYM_SEC,
                                "[COMMON]" );
                    continue;
               }
               
               symsecsheader = (Elf32_Shdr *)&memmap
                    [eheader.e_shoff +
                     (eheader.e_shentsize * thissym->st_shndx)];

               symsec = &secstrtab[symsecsheader->sh_name];
               mvwaddstr ( detpad, line++, DETAIL_SYM_SEC, symsec );

          }

          /* Check if we're doing a filter, if so, we're done! */
          if ( (secfilter) && (secfilter != &secnone) )
               break;


     }

     /* Easy as pie... */
     return ( detpad );
}

               
static void sel_sym ( int idx, unsigned char rtype )
{
     int k, i, lim;
     long adr;
     Elf32_Shdr *sheader;
     Elf32_Word relotype;
     Elf32_Rel *therel;
     struct elf_sections **sections_base, *sections_dummy[2];
     struct Selection * new_sel;
     char *secname, *reloctype;

     /* Get new reference ready */
     new_sel = dump_mkref ( );
     
     /* First we need to seek to the section header */
     /* The first entry is always suppoed to be "secnone" */
     /* which we always skip here and don't show on the list */
     sections_dummy[0] = NULL;
     sections_dummy[1] = secfilter;

     /* Check for any active filter and make sure sections_base */
     /* is set-up properly */
     if ( (secfilter) && (secfilter != &secnone) )
          sections_base = sections_dummy;
     else
          sections_base = relseclist;


     /* Now we need to seek */
     for ( k = 1; k < relseccount; k++ )
     {
          sheader = sections_base[k]->sheader;
          /* This is the relocation section header */

          /* How many relocations do we need to deal */
          /* with in this section? */
          lim = (int)sheader->sh_size / (int)sheader->sh_entsize;

          for ( i = 0; i < lim; i++ )
          {
               /* Check each one, only subtrack idx when */
               /* we get a match. */
               therel = (Elf32_Rel *)&memmap
                    [sheader->sh_offset + (sheader->sh_entsize * i)];

               if ( (unsigned char)therel->r_info != rtype )
                    continue;

               /* Got a hit */
               
               if ( idx <= 0 )
                    /* This is the one */
                    break;

               idx--;

          }

          if ( idx <= 0 )
               break;

          /* Check if we're doing a filter, if so, we're done! */
          if ( (secfilter) && (secfilter != &secnone) )
               break;

     }

     /* Set the appropriate parent section reference */
     new_sel->ref = sections_base[k]->adr;

     /* Load the relocation entry address */
     new_sel->adr = (long)(sheader->sh_offset +
           (sheader->sh_entsize * i));

     /* Set the type and size */
     if ( sheader->sh_type == SHT_RELA )
     {
          new_sel->size = (int)sizeof ( Elf32_Rela );
          new_sel->type = SEL_RELA;
          reloctype = "RELA";
     }
     else
     {
          new_sel->size = (int)sizeof ( Elf32_Rel );
          new_sel->type = SEL_REL;
          reloctype = "REL";
     }

     /* Finally we'll set the name appropriately */

     /* First load the section string index */
     secname = (char *)&memmap
          [eheader.e_shstrndx + sheader->sh_name];

     /* Print-out what we have to the name */
     snprintf ( new_sel->name, SEL_NAMEMAX,
                "%s: %s+0x%08x", reloctype, secname,
                therel->r_offset );

     sel_ref = new_sel;
     
     /* Switch over the the dump subsystem */
     actscr = dumpscr;

     dump_highlight ( UI_UNHIGHLIGHT, sel_adr, sel_size );
     sel_adr  = sel_ref->adr;
     sel_size = sel_ref->size;
     dump_highlight ( UI_HIGHLIGHT, sel_adr, sel_size );
     win_adr = sel_adr;

     /* Pass it over! */
     dump_base ( );

     actscr = reloscr;
     return;
}     


               
          
     


     

                    
          

          
          
          

     

               
          

          
          
               

     
          

     
