/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED
 */

/*
 *  XERIS Support Group
 *
 *  man%
 *  Human Relations Secretariat
 *
 *  itoa$
 *  Human-readable Integer Encoding
 *
 *  Service Desk
 *
 */

#include <xeris.h>

#include <man/man.h>
#include <man/std/man.h>

uint16_t man_itoa ( xdcf * cf )
{
     register uint8_t i, len;
     register unsigned char * writehead;
     uint8_t base, sgn;
     unsigned char buf[34];
     
     /* First, check that we have a buffer */
     /* and that it is the minimum size    */
     if ( (!cf->b.p) || (cf->b.s < 34) )
     {
          /* Reject */
          rpt_reject_ptr ( cf );
          return ( 0 );
     }

     /* Next check and process the service */
     /* request code */
     if ( cf->w.t.srq.rqc & 0xff00 )
          /* signed */
          sgn = 1;
     else
          sgn = 0;

     /* find our base */
     switch ( cf->w.t.srq.rqc & 0x00ff )
     {
          case SVC_MAN_ITOA_DEC:
               base = 10;
               break;

          case SVC_MAN_ITOA_OCT:
               base = 8;
               break;

          case SVC_MAN_ITOA_BIN:
               base = 2;
               break;

          default:
               base = 16;
               break;
     }

     /* Process sign if signed */
     if ( sgn && ( cf->w.t.srq.prm.p32[0] & 0x8000 ) )
     {
          /* Sign is negative. itostr does not */
          /* itself handle signed values, so   */
          /* we need to apply two's complement */
          /* and remember we are negative */
          
          cf->w.t.srq.prm.p32[0] = ~cf->w.t.srq.prm.p32[0];
          cf->w.t.srq.prm.p32[0]++;
          
          sgn = 2;
     }

     /* now let's get the actual conversion */
     itostr ( buf, cf->w.t.srq.prm.p32[0], base );

     /* And find how long it is */
     for ( len = 0; len < 34; len++ )
          if ( !buf[len] )
               break;

     /* Now we need to find our insertion point */
     writehead = (unsigned char *)cf->b.p;

     /* Take this opportunity to insert the sign */
     /* if needed */
     if ( sgn )
     {
          if ( sgn == 1 )
               *(writehead++) = '+';
          else
               *(writehead++) = '-';

          sgn = 1;
     }

     /* Basically we will set the head at the "end" of  */
     /* the number (least singificant digit), and write */
     /* backwards. If the first digit of the provided   */
     /* buffer is '0', then we will use that only.      */
     /* Otherwise, we will just allocate space for the  */
     /* converted integer */

     if ( *writehead == '0' )
          while ( *(writehead++) == '0' );
     else
          writehead = &writehead[len];

     /* We "overshot" by 1, in both cases.  */
     /* We use that to terminate the string */
     *(writehead--) = '\0';

     for ( i = (len - 1); i >= 0; i-- )
     {
          *writehead = buf[i];
          
          if ( writehead == &((unsigned char *)cf->b.p)[sgn] )
               /* We got to the start of the buffer! */
               break;

          writehead--;
     }

     /* Request is complete, wrap it up */
     cf->w.s = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}

