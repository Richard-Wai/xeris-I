/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All Rights Reserved.
 */

/*
 *  XERIS Support Group
 *
 *  f/man%
 *  Human Relations Secretariat
 *
 *  GENERIC Release 1
 *  Version 1.0
 *
 *  intern.c
 *  Internal helper functions
 */

#include <xeris.h>
#include <man/man.h>
#include <man/internal.h>

/* Insert a string into another at the first "``" marker */
void insrt ( unsigned char * buf, unsigned char * ins )
{
     /* This function is most often used for telemetry */
     /* translation. A standard insertion symbol for   */
     /* the purpose is two consecutive '`' characters  */
     /* insrt seeks to the first "``" encountered      */

     unsigned char rollout[BUFSTD];

     unsigned char * cutleft, * cutright;
     register uint8_t i;
     uint8_t lim;

     cutleft  = NULL;
     cutright = NULL;
     
     /* Seek buffer to "``" */
     for ( i = 0; i < BUFSTD; i++ )
     {
          if ( !buf[i] )
               /* unexpected end of string */
               return;
               
          if ( (buf[i] == '`') && (buf[i+1] == '`') )
          {
               /* bingo */
               cutleft = &buf[i];
               cutright = &buf[i+2];
               /* We dont want to go out of the   */
               /* input buffer on roll-out if the */
               /* insertion buffer is too big     */
               lim = BUFSTD - (i+2);
               break;
          }
     }

     if ( !cutleft )
          /* No symbol was found. */
          return;
     
     /* Roll-in the insertion string at cutleft */
     /* while we roll-out from cutright */
     rollout[0] = cutright[0];

     for ( i = 0; i < lim; i++ )
     {
          if ( (*ins) )
               /* stop copying if we reach null */
               *(cutleft++) = *(ins++);

          if ( !(*cutright) )
          {
               /* nothing left to roll-out */
               if ( !(*ins) )
                    break;
               else
                    continue;
          }

          rollout[i+1] = *(++cutright);
     }

     /* The roll-out needs to be finished to the end */
     /* of the string */

     lim = i;

     /* Roll-in the rollout */
     for ( i = 0; i < lim; i++ )
     {
          cutleft[i] = rollout[i];

          if ( !rollout[i] )
               break;
     }

     /* Ensure cap */
     cutleft[i] = '\0';

     return;
}

/* Convert a maximum 32-bit unsigned integer into human-readable */
/* form, with the provided radix */
extern void itostr ( unsigned char *buf, uint32_t inp, uint8_t base )
{
    register uint8_t i;
    register uint32_t acum, dgt;

    unsigned char keymap[17];

    /* First load the key map */
    ld_man ( STD_KEYMAP, keymap, 17 );
    if ( !keymap[0] )
         /* This should really not happen.. */
         /* fail passive! */
         return;

    if ( inp == 0 )             /* Don't even bother */
    {
         buf[0] = keymap[0];
         buf[1] = '\0';
         return;
    }

    /* Find the most singificant digit */
    /* we will start from there..      */
    for ( acum = base; ( inp >= acum ); acum *= base );


    /* We actually just over-ran the acumulator by one */
    /* iteration, which is why we start with a division */
    
    for ( i = 0; ( acum >= base ); i++ )
    {
         acum /= base;
         dgt = ( inp / acum );
         buf[i] = keymap[dgt];
         inp -= ( dgt * acum );

    }
    /* i is now "1 past" the end, so terminate both strings first,
        and back it up */
    buf[i] = '\0';

	return;
}

extern uint32_t strtoi ( unsigned char * buf, uint8_t base )
{
     /* This is for internal use, but bare in mind */
     /* we only recognise what's on the keymap     */
     
     register uint8_t i, j;
     register uint32_t rval;

     uint8_t maxw;
     unsigned char keymap[17];

     
     /* First load the key map */
     ld_man ( STD_KEYMAP, keymap, 17 );
     if ( !keymap[0] )
          /* This should really not happen.. */
          /* fail passive! */
          return ( 0 );
          
     rval = 0;

     /* Find the max possible width of the string */
     /* for a 32-bit unsigned integer */
     switch ( base )
     {
          case 2:
               maxw = 32;
               break;
               
          case 8:
               maxw = 11;
               break;

          case 10:
               maxw = 10;
               break;

          case 16:
               maxw = 8;
               break;

          default:
               /* Invalid */
               return ( 0 );
     }
                    
     
     /* Find end of string */
     /* We can only return a 32 bit number */
     /* so the input can't be more than 32 digits wide */
     for ( i = 0; i < maxw; i++ )
          if ( !buf[i+1] )
               break;
          

     for ( ; i < maxw; i-- )
     {
          for ( j = 0; j < base; j++ )
               if ( buf[i] == keymap[j] )
                    break;

          if ( j == base )
               /* invalid character for this base */
               return ( 0 );

          rval += ( j * ( base * i ) );
     }
               

     return ( rval );

    
}
     

/* Fill-out Case File ticket for a bad pointer config */
void rpt_reject_ptr ( xdcf * cf )
{
     cf->w.s = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_CB_CONFIG;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return;
}


