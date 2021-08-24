/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  ALL RIGHTS RESERVED
 */

/*
 *  XERIS Facilities Group
 *
 *  f/rdc%
 *  Recurrent Dispatch Commission
 *
 *  sub.c
 *  sub$ desk
 */

#include <xeris.h>
#include <xeris/siplex.h>
#include <xeris/f/rdc.h>
#include <xeris/s/rsv.h>

#include <f/rdc/rdc.h>

static void say_initreq ( xdcf * cf );

uint16_t sub ( xdcf * cf )
{
     /* Standard submission desk! */
     volatile struct tx_card tcard;
     xdcf cpcase;
     uint16_t popsec;

     /* Is this an event card? */
     if ( cf->w.s != XST_EVT )
     {
          /* No good. */
          cf->w.s = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Get a copy of the casefile for later reference */
     /* However note, the stamp is made by opsec%, so  */
     /* we need to save the stamp info now, before any */
     /* call to CDD */
     popsec = cf->s.sid;
     cf->b.p = (void *)&cpcase;
     cf->b.s = sizeof ( xdcf );
     xeris ( X_S_RSV );


     /* Attempt to pop a card */
     tcard.next = NULL;
     tcard.dispatch = NULL;

     tcard.op = ( TX_FREE | TX_POP );

     cf->b.p = (void *)&tcard;
     cf->b.s = sizeof ( struct tx_card );
     cf->w.d = X_F_RDC_TXP;
     /* push to transaction queue */
     xeris ( X_F_RDC );

     if ( !cf->b.p )
     {
          /* Not yet init'ed! */
          say_initreq ( cf );
          return ( 0 );
     }

     /* Key through the transactions */
     cf->w.d = X_F_RDC_TXE;
     while ( tcard.op != TX_COMMIT )
          xeris ( X_F_RDC );

     if ( !tcard.dispatch )
     {
          /* None left! */
          cf->w.s = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* So we have a dispatch card now. We just need   */
     /* to fill it out, then submit it to the dispatch */
     /* queue */
     tcard.dispatch->next  = NULL;
     tcard.dispatch->event.sid = popsec;
     tcard.dispatch->event.dsk = cpcase.w.t.evt.dsk;
     tcard.dispatch->event.ref = cpcase.w.t.evt.ref;
     tcard.dispatch->event.seq = cpcase.w.t.evt.seq;


     /* OK, try to push it to the dispatch queue */
     cf->b.p = (void *)&tcard;
     cf->b.s = sizeof ( struct tx_card );
     cf->w.d = X_F_RDC_TXP;
     tcard.op = ( TX_DISPATCH | TX_PUSH );
     tcard.next = NULL;
     xeris ( X_F_RDC );

     if ( !cf->b.p )
     {
          /* really unlikely.. */
          say_initreq ( cf );
          return ( 0 );
     }

     /* Key through */
     cf->w.d = X_F_RDC_TXE;
     while ( tcard.op != TX_COMMIT )
          xeris ( X_F_RDC );

     /* That should be it! */
     cf->b.p = NULL;
     cf->b.s = 0;

     cf->w.s = XST_ACP;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_ACCEPTED;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );

}
     

static void say_initreq ( xdcf * cf )
{
     cf->w.s = XST_UNA;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_I_INIT_REQUIRED;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;
     return;
}
