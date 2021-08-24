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
 *  stu$
 *  SOSES Translation Unit
 *
 *  Service Desk
 */


#include <xeris.h>

#include <man/man.h>
#include <man/internal.h>
#include <man/std/man.h>

static void pack_data ( xdcf * cf );

uint16_t man_stu ( xdcf * cf )
{
     unsigned char * bufmirror;
     unsigned char cxbuf[5];

     /* First we need to verify the validity   */
     /* of the ticket stub. Must be TLM or RPT */

     if ( !( (cf->w.s == XST_RPT) || (cf->w.s == XST_TLM) ) )
          /* We don't accept this, and do nothing */
          return ( 0 );

     /* Next we check the case board. The supplied buffer */
     /* must be indicated to be at least 83 bytes long    */
     /* This is in reference to the specification         */

     if ( ( !cf->b.p ) || ( cf->b.s < 83 ) )
     {
          /* Not acceptable, nullify pointer and return */
          cf->b.p = NULL;
          return ( 0 );
     }

     /* Attempt to load the SOSES string for the provided */
     /* spec/code pair..                                  */
     ld_soses ( cf->w.t.rpt.spec,
                cf->w.t.rpt.code,
                (unsigned char *)cf->b.p,
                cf->b.s );

     if ( ((unsigned char *)cf->b.p)[0] )
     {
          /* So we've loaded a code now, we can run it through     */
          /* the data packer, to insert any data that is requested */
          /* via any SOSES schema that may be in the SOSES         */
          /* translation */
          pack_data ( cf );

          /* That's it! */
          return ( 0 );
     }

     /* Looks like we had no luck finding a valid match  */
     /* for the provided code, so we instead must encode */
     /* it in the standard XSRES format */

     /* attempt to load */
     ld_man ( STD_XSRES, (unsigned char *)cf->b.p, cf->b.s );
     if ( !((unsigned char *)cf->b.p)[0] )
          /* That's not supposed to happen */
          return ( 0 );

     /* ok, now we have four inserts to make */
     /* (refer to specification) */

     /* So we dont need to keep doing a typecast */
     bufmirror = (unsigned char *)cf->b.p;
     
     /* 1. SID */
     itostr
     (
          cxbuf,
          (cf->w.s == XST_TLM)? cf->w.t.tlm.sec : cf->s.sid,
          16
     );
     insrt ( bufmirror, cxbuf );

     /* 2. SOSES Spec */     
     itostr ( cxbuf, cf->w.t.rpt.spec, 16 );
     insrt ( bufmirror, cxbuf );

     /* 3. SOSES Code */
     itostr ( cxbuf, cf->w.t.rpt.code, 16 );
     insrt ( bufmirror, cxbuf );

     /* 4. DGM/ADI dump */
     itostr ( cxbuf, cf->w.t.rpt.adi.adi32, 16 );
     insrt ( bufmirror, cxbuf );
     
     /* done */
     return ( 0 );
}

     
static void pack_data ( xdcf * cf )
{
     /* If we are here, it means the case board has a */
     /* SOSES translation code loaded into the board  */
     /* pointer, and a telemetry or report ticket.    */
     /* Our job is to process any Human Translation   */
     /* Schema datagram insertion codes that may be   */
     /* present in the SOSES string */

     register uint8_t i, j;
     unsigned char cbuf[34];
     uint8_t emark, omark, wsize, idx, clamp, guard;
     unsigned char *buf;
     xdcf tcase;

     buf = (unsigned char *)cf->b.p;

     /* We're more responsible than to actually */
     /* use an infinite loop. But there could be more */
     /* than one schema in the string, so we need to  */
     /* re-run until we can't find anymore. We can't  */
     /* safly handle more than 4.. */

     for ( guard = 0; guard < 4; guard++ )
     {
          /* Try to find (another?) a schema, and  */
          /* seek to it's starting point           */

          for ( i = 0; i < BUFSTD; i++ )
          {
               if ( buf[i] == '`' )
                    break;

               if ( !buf[i] )
                    return;
          }

          if ( i >= BUFSTD )
               return;

          /* It looks like we could have a schema here    */
          /* Mark the entry at first '`' and move to next */
          emark = ++i;

          /* Reset the temporary case file */
          tcase.b.p = (void *)cbuf;
          tcase.b.s = 34;

          tcase.w.s = XST_SRQ;


          if ( i >= BUFSTD )
               /* Assume _nothing_ */
               return;
          
          /* Note, according to the specification, anything */
          /* unexpected from this point invalidates the     */
          /* schema (the whole schema) */

          /* First, next character should indicate the      */
          /* conversion format */

          switch ( buf[i++] )
          {
               case 'b':
                    /* Unsigned binary */
                    tcase.w.t.srq.rqc
                         = (SVC_MAN_ITOA_UNSGN | SVC_MAN_ITOA_BIN);
                    break;

               case 'o':
                    /* Unsigned octal */
                    tcase.w.t.srq.rqc
                         = (SVC_MAN_ITOA_UNSGN | SVC_MAN_ITOA_OCT);
                    break;
               
               case 's':
                    /* Signed decimal */
                    tcase.w.t.srq.rqc
                         = (SVC_MAN_ITOA_SGN | SVC_MAN_ITOA_DEC);
                    break;

               case 'u':
                    /* Unsinged decimal */
                    tcase.w.t.srq.rqc
                         = (SVC_MAN_ITOA_UNSGN | SVC_MAN_ITOA_DEC);
                    break;

               case 'x':
                    /* Unsigned hex */
                    tcase.w.t.srq.rqc
                         = (SVC_MAN_ITOA_UNSGN | SVC_MAN_ITOA_HEX);
                    break;

               default:
                    /* Schema invalidated */
                    return;
          }

          if ( i >= BUFSTD )
               return;

          /* Now we try to sort-out the word size */
          switch ( buf[i++] )
          {
               case '1':
                    /* 8 bit */
                    wsize = 1;
                    break;
                    
               case '2':
                    /* 16 bit */
                    wsize = 2;
                    break;

               case '4':
                    /* 32 bit */
                    wsize = 4;
                    break;

               default:
                    /* Invalid */
                    return;

          }


          if ( i >= BUFSTD )
               return;
          
          /* Now we try to extract the index */
          switch ( buf[i++] )
          {
               case '0':
                    idx = 0;
                    break;
                    
               case '1':
                    if ( wsize > 2 )
                         /* invalid */
                         return;
                    idx = 1;
                    break;

               case '2':
                    if ( wsize > 1 )
                         return;
                    idx = 2;
                    break;

               case '3':
                    if ( wsize > 1 )
                         return;
                    idx = 3;
                    break;

               default:
                    /* invalid */
                    return;

          }

          if ( i >= BUFSTD )
               return;
          
          /* Now we take a moment to set the value */
          /* on the service ticket as per the last */
          /* two feilds */
          switch ( wsize )
          {
               case 1:
                    tcase.w.t.srq.prm.p32[0] =
                         (uint32_t)cf->w.t.tlm.dgm.dgm08[idx];
                    break;

               case 2:
                    tcase.w.t.srq.prm.p32[0] =
                         (uint32_t)cf->w.t.tlm.dgm.dgm16[idx];
                    break;

               case 4:
                    tcase.w.t.srq.prm.p32[0] =
                         cf->w.t.tlm.dgm.dgm32;
                    break;

               default:
                    /* This is impossible */
                    return;
          }

          /* Finally, we check for the option clamp field */

          clamp   = 0;
          cbuf[0] = 0;
          
          /* seek to end of clamp feild (if any) */
          for ( j = 0; (i < BUFSTD) && (j < 34); i++, j++ )
          {
               if ( buf[i] == '`' )
                    break;

               if ( !buf[i] )
                    /* Invalid */
                    return;

               cbuf[j] = buf[i];
          }

          if ( i == BUFSTD )
               /* overrun, this invalidates the schema */
               return;

          /* i should now be at the closing '`'. Set the */
          /* out-mark */
          omark = i;

          /* Attempt to get the indicated clamp size */
          if ( cbuf[0] )
               /* Should be base-10 */
               clamp = (uint8_t)strtoi ( cbuf, 10 );

          /* Fill the conversion buffer with the appropriate */
          /* number of zeros */
          for ( j = 0; j < clamp; j++ )
               cbuf[j] = '0';

          /* Cap it */
          cbuf[j] = 0;

          /* Now we should be ready to execute! */
          man_itoa ( &tcase );

          if ( tcase.w.s != XST_CPL )
               /* didn't work.. */
               return;

          /* Now what we need to do is delete the schema */
          /* so that it becomes just "``", and then run  */
          /* an insert operation on with the conversion  */
          /* buffer we just created */

          for ( i = emark, j = omark; j < BUFSTD; i++, j++ )
          {
               buf[i] = buf[j];

               if ( !buf[i] )
                    break;
          }

          buf[i] = 0;


          /* Last, but not least */
          insrt ( buf, cbuf );

          /* Now we try another! */

     }

     return;
}
          
