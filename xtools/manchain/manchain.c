/* MANCHIAN                                  */
/* Tool for managing MAN% message string     */
/* registration                              */
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

static void add_new
( uint16_t * chain, uint16_t idxa, uint16_t idxb, char * line, char single );

static void lookup
( uint16_t * chain, uint16_t idxa, uint16_t idxb, char single );

int main ( int argc, char ** argv )
{
     int mapfd, i;
     char single;
     
     uint16_t * addend, idxa, idxb, range;

     if ( argc < 4 )
     {
          puts ( "usage: file index index [string]\n\n" );
          exit ( 0 );
     }

     /* Initialize */
     
     mapfd = open ( argv[1], O_CREAT | O_RDWR | O_EXCL );

     if ( mapfd < 0 )
     {
          if ( errno == EEXIST )
          {
               mapfd = open ( argv[1], O_RDWR  );
               addend = mmap ( 0, 10240, PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_NOCORE, mapfd, 0 );
          }
          else
          {
               perror ( "unable to open" );
               exit ( -1 );
          }
     }
     else
     {
          ftruncate ( mapfd, 10240 );
          fchmod ( mapfd, 00666 );
          addend = mmap ( 0, 10240, PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_NOCORE, mapfd, 0 );

          memset ( addend, 0, 10240 );
          *addend = (2 * 16) + 1;
          fsync ( mapfd );
     }

     if ( argv[2][0] == '-' )
     {
          single = 1;
          idxa = 0;
          idxb = (uint16_t)strtol ( argv[3], NULL, 16 );
     }
     else
     {
          single = 0;
          idxa = (uint16_t)strtol ( argv[2], NULL, 16 );
          idxb = (uint16_t)strtol ( argv[3], NULL, 16 );
     }

     range = 1;
     
     if ( argc > 4 )
     {
          if ( !strcmp ( argv[4], "-r" ) )
          {
               range = (uint16_t)strtol ( argv[5], NULL, 16 );
          }
          else
          {
               add_new ( addend, idxa, idxb, argv[4], single );
          }
     }

     fsync ( mapfd );
     
     for ( i = 0; i < range; i++ )
          lookup ( addend, idxa, idxb + i, single );


     close ( mapfd );

     exit ( 0 );
}


static void add_new
( uint16_t * chain, uint16_t idxa, uint16_t idxb, char * line, char single )
{
     int i;
     
     unsigned char * target;
     uint16_t comp;
     uint16_t * mark;

     size_t len;

     uint8_t idx[4];

     len = strnlen ( line, 1024 );

     /* Scan for formatting */
     
     for ( i = 0; i < len; i++ )
     {
          if ( line[i] == '^' )
          {
               switch ( line[i + 1] )
               {
                    case 'n':
                         line[i    ] = '\r';
                         line[i + 1] = '\n';
                         break;

                    case 'l':
                         line[i    ] = '\n';
                         line[i + 1] = '\n';

                    default:
                         break;
               }
          }
     }

     len++;                     /* account for null in size */



     mark = &chain[1];
     target = NULL;

     if ( !single )
     {
          /* First try to seek to idxa */
          idx[0] = ((uint8_t)(idxa >> 12)) & 0x0f;
          idx[1] = ((uint8_t)(idxa >> 8)) & 0x0f;
          idx[2] = ((uint8_t)(idxa >> 4)) & 0x0f;
          idx[3] = ((uint8_t)idxa) & 0x0f;
          
          /* First pass */
          for ( i = 0; i < 4; i++ )
          {
               if ( !mark[idx[i]] )
               {
                    mark[idx[i]] = chain[0];
                    
                    /* we need to make our own */
                    chain[0] += 16;
                    memset ( &chain[mark[idx[i]] + 1], 0, 16 * 2 );
               }
               
               mark = &chain[mark[idx[i]] + 1];
          }

     }
     
     idx[0] = ((uint8_t)(idxb >> 12)) & 0x0f;
     idx[1] = ((uint8_t)(idxb >> 8)) & 0x0f;
     idx[2] = ((uint8_t)(idxb >> 4)) & 0x0f;
     idx[3] = ((uint8_t)idxb) & 0x0f;
          
     for ( i = 0; i < 4; i++ )
     {
          if ( !mark[idx[i]] )
          {
               mark[idx[i]] = chain[0];
               
               /* we need to make our own */
               if ( i < 3 )
               {
                    chain[0] += 16;
                    mark = &chain[mark[idx[i]] + 1];
                    memset ( mark, 0, 16 * 2 );


               }
               else
               {
                    target = (unsigned char *)&chain[mark[idx[i]] + 1];
                    chain[0] += ( len / 2 ) + ( len % 2 );
                    memset ( target, 0, len );
                    strncpy ( target, line, len );
               }
          }
          else
          {
               mark = &chain[mark[idx[i]] + 1];
          }
                             
     }

     if ( !target )
          puts ( "String already registered" );

     return;
}

static void lookup
( uint16_t * chain, uint16_t idxa, uint16_t idxb, char single )
{
     unsigned char buffer[120];
     uint16_t * mark, * base;

     unsigned char *pull;

     int i;
     uint8_t idx[4];


     mark = &chain[1];
     base = mark;
     pull = NULL;

     if ( !single )
     {
          idx[0] = ((uint8_t)(idxa >> 12)) & 0x0f;
          idx[1] = ((uint8_t)(idxa >> 8)) & 0x0f;
          idx[2] = ((uint8_t)(idxa >> 4)) & 0x0f;
          idx[3] = ((uint8_t)idxa) & 0x0f;
          
          for ( i = 0; i < 4; i++ )
          {
               if ( mark[idx[i]] )
               {
                    mark = &base[mark[idx[i]]];
               }
               else
               {
                    printf ( "%04X-%04X: [EMPTY]\n", idxa, idxb );
                    return;
               }
               
          }
     }

     idx[0] = ((uint8_t)(idxb >> 12)) & 0x0f;
     idx[1] = ((uint8_t)(idxb >> 8)) & 0x0f;
     idx[2] = ((uint8_t)(idxb >> 4)) & 0x0f;
     idx[3] = ((uint8_t)idxb) & 0x0f;
     
     for ( i = 0; i < 4; i++ )
     {
          if ( mark[idx[i]] )
          {
               if ( i < 3 )
                    mark = &base[mark[idx[i]]];
               else
                    pull = (unsigned char *)&base[mark[idx[i]]];
          }
          else
          {
               break;
          }
     }

     if ( !pull )
     {
          printf ( "%04X-%04X: [EMPTY]\n", idxa, idxb );
     }
     else
     {
          strncpy ( buffer, pull, 120 );
          printf ( "%04X-%04X: %s\n", idxa, idxb, buffer );
     }

     return;
}
          

                         


               
     
