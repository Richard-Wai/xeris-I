/* CATHEX                                    */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* This utility concatenates a number of intel HEX files into */
/* a single one, sequentially. This is used to put together   */
/* an target image with all of the node's Secretariats        */

#include <stdio.h>


/* Data section, the full flash memory map */

unsigned char mmap[0x7fff];

#define FLASH_START 0x100
#define FLASH_END   0x7fff

#define REC_DATA    0x00
#define REC_EOF     0x01
#define REC_ESA     0x02
#define REC_SSA     0x03
#define REC_ELA     0x04
#define REC_SLA     0x05

static int loadhex ( FILE *, const char * );
static void writehex ( FILE *, int extent );

int main ( int argc, const char **argv )
{
     register unsigned int i;
     FILE *hexfile;

     int btotal = 0, bfile;

     if ( argc < 2 )
     {
          printf ( "No input files!\n\n" );
          return ( 0 );
     }

     printf ( "Clearing memory map (%i KB)... ", ((FLASH_END+1) / 1024 ) );

     for ( i = 0; i <= FLASH_END; i++ )
          mmap[i] = 0xff;

     printf ( "done." );

     /* stage 1, load each hex file into the mmap */
     for ( i = 1; i < argc; i++ )
     {
          printf ( "\n", argv[i] );
          hexfile = fopen ( argv[i], "r" );
          if ( !hexfile )
          {
               printf ( "Failed to access.\t\t%s", argv[i] );
               continue;
          }

          bfile = loadhex ( hexfile, argv[i] );
          fclose ( hexfile );

          if ( bfile > btotal )
               btotal = bfile;
          
     }

     /* stage 2, re-encode it all */
     if ( !( hexfile = fopen ( "packed.hex", "w" ) ) )
     {
          printf ( "\nERROR! Cannot open output file!\n" );
          return ( -1 );
     }

     printf ( "\n\nWriting new file...\n" );
     writehex ( hexfile, btotal );
     fclose ( hexfile );

     return ( 0 );
}


static int loadhex ( FILE * hexfile, const char * param )
{
     unsigned char
          cstart_code[2+1],
          cbyte_count[2+1],
          caddress[4+1],
          crecord_type[2+1],
          cchecksum[2+1],
          readbyte[2+1],
          checkbyte,
          checksum;

     int i, next;
     
     int
          start_code,
          byte_count,
          record_type,
          newbyte;

     unsigned int base = 0, offset = 0, mark = 0;

     /* Lets get to it! */
     while ( (next = fgetc ( hexfile )) != EOF )
     {
          
          /* This should be the start of the line */
          if ( next != ':' )
               break;


          checksum = 0;
          
          /* read in the byte_count */
          if ( (next = fgetc ( hexfile )) == EOF )
               break;
          cbyte_count[0] = (unsigned char)next;
          
          if ( (next = fgetc ( hexfile )) == EOF )
               break;
          cbyte_count[1] = (unsigned char)next;
          cbyte_count[2] = '\0';

          /* Convert the value */
          sscanf ( cbyte_count, "%X", &byte_count );
          checksum += (unsigned char)byte_count;

          /* Get the address */
          for ( i = 0; i < 4; i++ )
          {
               next = fgetc ( hexfile );

               if ( next == EOF )
                    break;

               caddress[i] = (unsigned char)next;
          }
          caddress[4] = '\0';

          if ( i < 4 )
               break;

          /* get record type!  */
          if ( (next = fgetc ( hexfile )) == EOF )
               break;
          crecord_type[0] = (unsigned char)next;
          
          if ( (next = fgetc ( hexfile )) == EOF )
               break;
          crecord_type[1] = (unsigned char)next;
          crecord_type[2] = '\0';

          sscanf ( crecord_type, "%X", &record_type );
          checksum += (unsigned char)record_type;

          if ( record_type == REC_EOF )
          {
               /* End of file */
               printf ( "\r[0x%04X:0x%04X] %i bytes \t%s ",
                        mark, offset, (offset - mark), param );
               return ( offset );
          }

          if ( record_type == REC_DATA )
          {
               sscanf ( caddress, "%X", &offset );

               if ( !mark )
                    mark = offset;

               printf ( "\r[0x%04X:0x%04X] %i bytes \t%s ",
                        mark, offset, (offset - mark), param );
               
               /* break-down the address for checksumming */
               crecord_type[0] = caddress[2];
               crecord_type[1] = caddress[3];
               sscanf ( crecord_type, "%X", &record_type );
               checksum += (unsigned char)record_type;

               caddress[2] = '\0';
               sscanf ( caddress, "%X", &record_type );
               checksum += (unsigned char)record_type;
               
               /* data, we're on */
               for ( i = 0; i < byte_count; i++ )
               {
                    readbyte[0] = (unsigned char)fgetc ( hexfile );
                    readbyte[1] = (unsigned char)fgetc ( hexfile );
                    readbyte[2] = '\0';

                    sscanf ( readbyte, "%X", &newbyte );
                    mmap[offset++] = (unsigned char)newbyte;
                    checksum += newbyte;

               }

               /* Do checksum */
               checkbyte = (unsigned char)checksum;
               checkbyte = ~checkbyte;
               checkbyte++;

               cchecksum[0] = fgetc ( hexfile );
               cchecksum[1] = fgetc ( hexfile );
               cchecksum[2] = '\0';

               sscanf ( cchecksum, "%X", &i );
               checksum = (unsigned char)i;

               if ( checksum != checkbyte )
                    printf ( " BAD CHECKSUM.\n" );

               next = fgetc ( hexfile );

          }
          else
          {
               do
               {
                    next = fgetc ( hexfile );
               }while ( (next != '\r') && (next != '\n') && (next != EOF) );

               if (next == EOF)
                    break;
          }
                         
          /* Finally, this should be the end of the line */


          if ( next != '\n' )
          {
               if ( next != '\r' )
                    break;
               
               next = fgetc ( hexfile );
               
               if ( next != '\n' )
                    break;
          }
          
     }


     /* If we get here, something was wrong with the file */
     printf ( "FILE INVALID.\n" );

     return ( 0 );
}

static void writehex ( FILE * hexfile, int extent )
{
     int i, address, lim;
     unsigned char outbyte, checksum, buffer[10];

     address = 0;

     while ( address < extent )
     {
          fprintf ( hexfile, ":" );

          /* Number of bytes we will output */
          if ( (address + 0x10) >= extent )
               lim = extent - address;
          else
               lim = 0x10;

          fprintf ( hexfile, "%02X", lim );
          checksum = (unsigned char)lim;

          /* address (16-bit) */
          fprintf ( hexfile, "%04X", address );
          checksum += (unsigned char)( (address & 0xff00) >> 8 );
          checksum += (unsigned char)(address & 0x00ff);

          /* Data record type */
          fprintf ( hexfile, "00" );

          /* Put out the data! */
          for ( i = 1; i <= lim; i++ )
          {
               fprintf ( hexfile, "%02X", mmap[address] );
               checksum += mmap[address];

               printf ( "\r[0x0000:0x%04X]: %i bytes\tpacked.hex",
                        address, address );
               address++;
          }

          /* Lastly, do the checksum */
          checksum = ~checksum;
          checksum++;

          fprintf ( hexfile, "%02X\r\n", checksum );

     }

     /* Print-out the "pre-scripted" Start segment */
     //fprintf ( hexfile, ":040000030000000009\r\n" );

     /* Print-out the "pre-scripted" EOF line */
     fprintf ( hexfile, ":00000001FF\r\n" );

     printf ( "\nTotal image: %i KB.\n\n",
              ((address - 1) / 1024) );

     return;
}
     
          
          

          

     

     

     

                    



                             
                    
                    

          

          
         
          

     

     
     
     
