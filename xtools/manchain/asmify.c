/* asmify                                    */
/* Converts raw binary input into assembler  */
/* word directives (avr)                     */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main ( int argc, char ** argv )
{

     int infd;
     uint16_t buf, last, i;

     infd = 1;

     if ( argc > 1 )
     {
          infd = open ( argv[1], O_RDONLY );

          if ( infd < 0 )
          {
               perror ( "unable to open input" );
               exit ( -1 );
          }
     }

     /* We skip this part too */
     if ( read ( infd, &last, 2 ) < 2 )
     {
          perror ( "read error" );
          exit ( -1 );
     }


     /* ok lets do this */
     for ( i = 1; i <= last; i++ )
     {
          
          if ( read ( infd, &buf, 2 ) < 2 )
          {
               perror ( "read error" );
               exit ( -1 );
          }
          
          printf ( ".word 0x%04x\n", buf );

     }

     exit ( 0 );
}
