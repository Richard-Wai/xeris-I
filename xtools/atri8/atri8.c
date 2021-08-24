/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <curses.h>

#include "globl.h"
#include "ui.h"
#include "elf.h"
#include "dump.h"
#include "relo.h"
#include "pack.h"
#include "sim.h"


// Internal operations
long mem_load_elf ( );
long mem_load_bin ( );
void mem_proc_elf ( );


int main ( int argc, char **argv )
{

     char * openfile;
     char rand_meet[9];

     elfmain = NULL;
     outfile = NULL;
     sel_list = NULL;
     sel_ref = NULL;
     memmap = NULL;
     isLittleEndian = 1;
     symstroff = 0;

     unsigned int cpy = 0;

     if ( argc > 1 )
     {
          openfile = argv[1];

          if ( argc >= 3 )
          {
               /* Now check for auto arguments */
               if ( !strncmp( argv[2], "-a", 2 ) )
               {
                    /* We expect:  */
                    /* -a [sid] [cdd] [usec] [dsec] [bpri] */
                    /* (-t [tx] -i [isr] [irq] */
                    if ( argc < 8 )
                    {
                         printf
                  ( "usage: -a [pack] [sid] [cdd] [usec] [dsec] [bpri]\n" );
                         return ( -1 );
                    }


                    /* Add to image - basically pack */
                    Auto.at = 0xff;
                    Auto.img  = argv[1];


                    Auto.pkg = argv[3];

                    sscanf ( argv[4], "%x", &cpy );
                    Auto.sid = cpy;
                    
                    Auto.cdd = argv[5];
                    
                    sscanf ( argv[6], "%x", &cpy );
                    Auto.usec = cpy;
                    
                    sscanf ( argv[7], "%x", &cpy );
                    Auto.dsec = cpy;

                    sscanf ( argv[8], "%x", &cpy );
                    Auto.bpri = cpy;

                    if ( argc > 9 )
                    {
                         if ( !strncmp ( argv[9], "-t", 2 ) )
                         {
                              sscanf ( argv[10], "%x", &cpy );
                              Auto.tx = cpy;

                              if ( argc > 11 )
                              {
                                   if ( !strncmp ( argv[11], "-i", 2 ) )
                                   {
                                        Auto.isr = argv[10];
                                        sscanf ( argv[13], "%i", &cpy );
                                        Auto.irq = cpy;
                                   }
                              }
                              
                         }
                         else if ( !strncmp ( argv[9], "-i", 2 ) )
                         {
                              Auto.isr = argv[10];
                              sscanf ( argv[11], "%i", &cpy );
                              Auto.irq = cpy;
                         }
                    }

                    /* Set up a random rodevous before */
                    /* forking off */
                    snprintf ( rand_meet, 9, "%X",
                               ( arc4random() % ((unsigned)RAND_MAX + 1) ) );
                    Auto.cpoint = rand_meet;

                    /* Explicitly kill all "windows" */
                    dumpscr = NULL;
                    infoscr = NULL;
                    regscr  = NULL;
                    simscr  = NULL;
                    reloscr = NULL;
                    actscr  = NULL;

                    // Giv 'er
                    elfmain = NULL;
                    
                    if ( !fork ( ) )
                    {
                         /* We are child, which means we are the sim */
                         Auto.img = Auto.pkg;
                         Auto.pkg = NULL;
                         
                         elf_open_object ( Auto.img );
                         sim_base ( );
                         return ( 0 );
                    }
                    else
                    {
                         /* Give a chance for the master */
                         /* to set-up the socket */
                         usleep ( 50000 );
                         
                         elf_open_object ( Auto.img );
                         if ( elfmain )
                         {
                              mem_load_elf ( );
                              pack_base ( );
                         }
                         else
                         {
                              perror ( "unable to open:" );
                         }
                         return ( 0 );
                    }
                    
               }
               
          }
     }


     // set up the UI environment, if any
     initscr ( );
     noecho ( );
     clear ( );
     refresh ( );
     getmaxyx ( stdscr, termy, termx );
     termy--;
     termx--;
     
     // turn on terminal key assignments
     keypad ( stdscr, TRUE );
     
     // create all the parent windows
     if ( !( homescr = newwin ( termy + 1, termx + 1, 0, 0 ) ) )
     {
          endwin ( );
          puts ( "Memory allocation error.\n\n" );
          return ( -1 );
     }
     
     UI_inithome ( );
     wrefresh ( homescr );
     
     dumpscr = dupwin ( homescr );
     infoscr = dupwin ( homescr );
     regscr  = dupwin ( homescr );
     simscr  = dupwin ( homescr );
     reloscr = dupwin ( homescr );
     
     if
          (
               ( dumpscr == NULL ) ||
               ( infoscr == NULL ) ||
               ( regscr  == NULL ) ||
               ( simscr  == NULL ) ||
               ( reloscr == NULL )
               )
     {
          endwin ( );
          puts ( "Could not allocate main windows. \n\n" );
          return ( -1 );
     }
     
     relo_init ( );
     
     wrefresh ( homescr );

     UI_homebase ( openfile );
          
 quit:

     if (Auto.at)
          return ( 0 );
     
     clear ( );
     refresh ( );
     endwin ( );
     puts ( "\n" );
     
     return ( 0 );
     
}

/*

    Function Definitions

*/

/* UI Procedures */

void UI_inithome ( void )
{
    wclear ( homescr );
    UI_fwin ( homescr,
              "atri8: XERIS/APEX Secretariat Processing Tool",
              "" );
    touchwin ( homescr );
    return;
}

void UI_homebase ( char * openfile )
{
     /* Our main area! */
     char filename[1024];
     long dump_adr = 0, sel_adr = 0;
     int sel_size = 0, task = 1;
     int smay, smax;

     int i, j;

     struct MenuItemList menu[6];
    
     char * MainMenu[6] =
     {
          "Open new ELF object file",
          "HEX Analysis Tool",
          "ELF Relocation Analysis",
          "Host Package",
          "Virtual ATRIA",
          "Exit Program"
     };

     actscr = homescr;
     getmaxyx ( actscr, smay, smax );
     smay--, smax--;



     /* Initialize menu list */
     for ( i = 0; i < 6; i++ )
     {

          menu[i].item = MainMenu[i];
          /* ASCII Hack */
          menu[i].key  = (int)(0x31 + i);
     }

     if ( openfile )
     {
          strncpy ( filename, openfile, 1024 );
          filename[1023] = '\0';
          elf_open_object ( filename );
          if ( elfmain )
          {
               mem_load_elf ( );
               dump_init ( );
          }
     }
     else
     {
          filename[0] = '\0';
     }
     
     while ( 1 )
     {
          /* Refresh the status bar */
          wattrset ( actscr, A_REVERSE );
          mvwaddstr ( actscr, smay, 1, "Select a Task. . ." );

          if ( elfmain )
               wattrset ( actscr, A_STANDOUT );
          else
               wattrset ( actscr, A_REVERSE );
          
          for ( i = 1; i <= 31; i++ )
               mvwaddch ( actscr, smay, smax - i, ' ' );

          /* seek to end of file name */
          for ( i = 0; i < 1024; i++ )
               if ( !filename[i] )
                    break;

          if ( i < 30 )
               i = 0;
          else
               i -= 30;
          
          mvwaddstr ( actscr, smay, (smax - 31), &filename[i] );

          if ( !elfmain )
          {
                  mvwaddstr
               ( actscr, smay, (smax - 31), "  [No Object File Open]" );
          }

          wrefresh ( actscr );
          
          /* Do the actual menu! */
          switch ( UI_static_menu ( actscr, (smax/4), menu, 6, task ) )
          {
               case '1':
                    task = 1;
                    /* Open new file */
                    if ( elfmain )
                    {
                         fclose ( elfmain );
                         dump_kill ( );
                         free ( (void *)memmap );
                         filename[0] = '\0';
                         elfmain = NULL;
                    }
                    elf_open_object ( filename );
                    
                    /* Redo dumps */
                    mem_load_elf ( );
                    dump_init ( );
                    /* g2g */
                    break;

               case '2':
                    /* Elf Analysis Tool */
                    task = 2;
                    if ( !elfmain )
                    {
                         UI_msgbox
                         (
                              " No Object Opened ",
                              "An ELF object must be opened to use this tool.",
                              "Press Any Key to Return"
                         );
                         break;
                    }

                    dump_base ( );


                    break;

               case '3':
                    /* ELF Relocation Analysis */
                    task = 3;
                    if ( !elfmain )
                    {
                         UI_msgbox
                         (
                              " No Object Opened ",
                              "An ELF object must be opened to use this tool.",
                              "Press Any Key to Return"
                         );
                         break;
                    }

                    relo_base ( );
                    break;

               case '4':
                    /* Host package */
                    task = 4;
                    if ( !elfmain )
                    {
                         UI_msgbox
                         (
                              " No Object Opened ",
                              "An ELF object must be opened to use this tool.",
                              "Press Any Key to Return"
                         );
                         break;
                    }

                    pack_base ( );
                    break;

               case '5':
                    /* ATRIA Sim */
                    task = 5;

                    sim_base ( );
                    break;
                    

               case '6':
                    /* Exit */
                    if ( elfmain )
                         fclose ( elfmain );
                    dump_kill ( );
                    return;
                    

               default:
                    break;
          }
                    
          actscr = homescr;
          touchwin ( actscr );
          wrefresh ( actscr );                    


     }

}

          
long mem_load_elf ( void )
{
     long i;
     struct Selection * sel_finger;
     
     if ( !elfmain )
          return 0;

     /* go to end of file and see how many bytes we need */
     fseek ( elfmain, 0, SEEK_END ); 
     max_adr = ftell ( elfmain );

     /* Attempt a malloc on the region */
     memmap = (unsigned char *)malloc ( (size_t)(max_adr+1) );

     if ( !memmap )
          err_nomem ( );
     
     /* Ok, just proceedurally load in the file */
     rewind ( elfmain );
     for ( i = 0; i <= max_adr; i++ )
          memmap[i] = (unsigned char)fgetc ( elfmain );

     if ( max_adr < (sizeof ( Elf32_Ehdr ) - 1) )
          return max_adr;
     

     /* Load in headers and populate selection entries */
     /* Clear selection list */
     if ( sel_list )
     {
          /* First entry stays! */
          if ( sel_list->next )
          {
               sel_finger = sel_list->next;
               sel_finger->prev = NULL;

               /* Seek to end */
               while ( sel_finger->next )
                    sel_finger = sel_finger->next;

               /* Back up burning it all behind */
               while ( sel_finger->prev )
               {
                    sel_finger = sel_finger->prev;
                    free ( sel_finger->next->name );
                    free ( sel_finger->next );
               }

               free ( sel_finger );
               sel_finger = NULL;
               sel_list->next = NULL;
          }

     }
     else
     {
          /* Make the first section.. should become the */
          /* elf header */
          sel_list = (struct Selection *)
               malloc ( sizeof ( struct Selection ) );

          if ( !sel_list )
               err_nomem ( );
          
          /* Malloc the name */
          sel_list->name = (char *)malloc ( sizeof ( char ) * SEL_NAMEMAX );
          if ( !sel_list->name )
               err_nomem ( );
     }

     /* Set up as the active reference */
     sel_ref = sel_list;

     /* Set up a default */
     sel_ref->prev = NULL;
     sel_ref->next = NULL;
     sel_ref->adr  = 0;
     sel_ref->size = 1;
     sel_ref->type = SEL_DEFAULT;

     strncpy ( sel_ref->name, sel_type_list[SEL_DEFAULT], SEL_NAMEMAX );
     
     memcpy ( (void *)&eheader, (void *)memmap, sizeof ( Elf32_Ehdr ) );

     if
     (
          (eheader.e_ident[EI_MAG0] != ELFMAG0) ||
          (eheader.e_ident[EI_MAG1] != ELFMAG1) ||
          (eheader.e_ident[EI_MAG2] != ELFMAG2) ||
          (eheader.e_ident[EI_MAG3] != ELFMAG3)
     )
          return max_adr;
     

     if ( eheader.e_ident[EI_CLASS] != ELFCLASS32 )
          return max_adr;

     if ( eheader.e_ident[EI_DATA] == ELFDATA2LSB )
          isLittleEndian = 1;
     else
          isLittleEndian = 0;

     /* So far so good, let's set up the default reference */
     /* as an elf header */
     

     /* Create entry for elf header */
     sel_ref->next =
          (struct Selection *)malloc ( sizeof ( struct Selection ) );

     if ( !sel_list->next )
          err_nomem ( );

     sel_ref->next->name =
          (char *)malloc ( sizeof ( char ) * SEL_NAMEMAX );
     if ( !sel_ref->next->name )
          err_nomem ( );
     strncpy
     (
          sel_ref->next->name,
          sel_type_list[SEL_EHEADER],
          SEL_NAMEMAX
     );

     sel_ref->next->prev = sel_ref;
     sel_ref->next->next = NULL;
     sel_ref->next->adr  = 0;
     sel_ref->next->size = (int)sizeof ( Elf32_Ehdr );
     sel_ref->next->type = SEL_EHEADER;


     return max_adr;
          
}





     


               
