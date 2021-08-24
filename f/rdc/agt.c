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
 *  agt.c
 *  agt$ desk
 */

#include <xeris.h>
#include <xeris/siplex.h>
#include <xeris/f/rdc.h>
#include <xeris/s/clr.h>

#include <f/rdc/rdc.h>

static uint16_t agt_lead ( xdcf * );
static uint16_t agt_exec ( xdcf * );

uint16_t agt ( xdcf * cf )
{
     /* f/rdc%/agt$ - Agency Desk */
     /* The Agency Desk disatches incoming case files */
     /* to either a lead agent, or an executive agent */
     
     /* Lead Agents are registered on cyclic shots.   */
     /* Their job is to pull dispatch cards off the   */
     /* dispatch queue, and then attempt to dispatch  */
     /* an Executive Agent on a public command shot   */

     /* Executive Agents are registered on generic    */
     /* command shots, and are responsible for        */
     /* actually delivering the event ticket to the   */
     /* dispatch card registrant */

     /* The difference is, Lead Agents have empty     */
     /* board pointers, Execs have their dispatch     */
     /* assignment loaded to the board                */

     if ( !cf->b.p )
          /* Lead Agent */
          return agt_lead ( cf );

     return agt_exec ( cf );
}

static uint16_t agt_lead ( xdcf * cf )
{
     /* We don't modify the case file at all, just */
     /* check the dispatch queue and try to assign */
     /* it to an Executive Agent */

     volatile struct tx_card         tcard;
     xdcf                            dcase;

     tcard.op       = ( TX_DISPATCH | TX_POP );
     tcard.next     = NULL;
     tcard.dispatch = NULL;

     /* First attempt to push the transaction card */
     cf->b.p = (void *)&tcard;
     cf->b.s = sizeof ( struct tx_card );
     cf->w.d = X_F_RDC_TXP;
     xeris ( X_F_RDC );

     if ( !cf->b.p )
     {
          /* So this means that we can't submit  */
          /* the transaction card. This most     */
          /* likely means that rdc% has not yet  */
          /* been initialized. We will leave the */
          /* report on the case file, it doesn't */
          /* effect us */
          cf->w.d = X_F_RDC_AGT;
          return ( 0 );
     }

     cf->b.p = NULL;
     cf->b.s = 0;

     /* Now we key through until ours is done */
     cf->w.d = X_F_RDC_TXE;
     while ( tcard.op != TX_COMMIT )
          xeris ( X_F_RDC );

     /* Good, let's see what we got! */
     if ( !tcard.dispatch )
     {
          /* we got nothin. No dispatches on the que! */
          cf->b.p = NULL;
          cf->b.s = 0;
          cf->w.d = X_F_RDC_AGT;
          return ( 0 );
     }


     /* Cool, we finally got one. Try to assign it */
     dcase.b.p = (void *)tcard.dispatch;
     dcase.b.s = sizeof ( struct dispatch_card );

     dcase.w.d = X_F_RDC_AGT;
     dcase.w.s = XST_EVT;

     /* Set up for SIPLEX% */
     cf->b.p = (void *)&dcase;
     cf->b.s = sizeof ( xdcf );
     cf->w.d = X_SIPLEX_SUB;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SUB_LEAS;
     cf->w.t.srq.prm.p16[0] = X_F_RDC;
     cf->w.t.srq.prm.p16[1] = SHOT_CMD;
     cf->w.t.srq.prm.p16[2] = 0;        /* No permit */

     /* Go! */
     xeris ( X_SIPLEX );

     if ( cf->w.s != XST_ACP )
     {
          /* Nope... no can do.. Put the dispatch card */
          /* back and try the whole thing again later  */
          tcard.op = ( TX_DISPATCH | TX_PUSH );
          tcard.next = NULL;
          
          cf->b.p = (void *)&tcard;
          cf->b.s = sizeof ( tcard );
          cf->w.d = X_F_RDC_TXP;
          xeris ( X_F_RDC );

          if ( !cf->b.p )
               return ( 0 );

          cf->b.p = NULL;
          cf->b.s = 0;
          cf->w.d = X_F_RDC_TXE;

          /* Key through */
          while ( tcard.op != TX_COMMIT )
               xeris ( X_F_RDC );

     }

     /* All done this round, back to it! */
     cf->b.p = NULL;
     cf->b.s = 0;
     return ( 0 );
}


uint16_t agt_exec ( xdcf * cf )
{
     /* We are now an Executive Agent, and our job is    */
     /* fairly simple. We just need to load in the event */
     /* ticket into the case file, and send it off. Then,*/
     /* when it comes back, check for confirmation       */

     volatile struct tx_card tcard;

     /* First check for any issue with the case board */
     if ( cf->b.s != sizeof ( struct dispatch_card ) )
     {
          /* For telemetry */
          cf->b.p = NULL;
          cf->b.s = 0;
          cf->w.s = XST_TLM;
          cf->w.t.tlm.spec = XSR_OP;
          cf->w.t.tlm.code = XSR_OP_F_CB_CONFIG;
          return ( 0 );
     }

     tcard.next     = NULL;
     tcard.dispatch = (struct dispatch_card *)cf->b.p;


     /* Copy out the Event Card */
     cf->b.p = NULL;
     cf->b.s = 0;
     
     cf->w.s         = XST_EVT;
     cf->w.t.evt.sid = tcard.dispatch->event.sid;
     cf->w.t.evt.dsk = tcard.dispatch->event.dsk;
     cf->w.t.evt.ref = tcard.dispatch->event.ref;
     cf->w.t.evt.seq = tcard.dispatch->event.seq;

     cf->w.d = cf->w.t.evt.dsk;

     /* Send it off! */
     xeris ( cf->w.t.evt.sid );

     /* Check for confirmation */
     if
     (
          ( cf->w.s         == XST_EVT                   ) &&
          ( cf->w.t.evt.sid == tcard.dispatch->event.sid )
     )
     {
          /* Cool, we have confirmation. First update the file */
          tcard.dispatch->event.dsk = cf->w.t.evt.dsk;
          tcard.dispatch->event.ref = cf->w.t.evt.ref;
          tcard.dispatch->event.seq = cf->w.t.evt.seq;

          /* Now attempt to put it back on the dispatch */
          /* queue */
          tcard.op = TX_DISPATCH;
     }
     else
     {
          /* Not confirmed, so we drop it from the queue  */
          tcard.op = TX_FREE;
     }

     /* Attempt the transaction */
     tcard.op |= TX_PUSH;     
     cf->b.p   = (void *)&tcard;
     cf->b.s   = sizeof ( struct tx_card );
     cf->w.d   = X_F_RDC_TXP;
     xeris ( X_F_RDC );
     

     if ( !cf->b.p )
     {
          /* This should serriosuly be 100% impossible */
          /* but that's what everyone says */
          cf->b.p = NULL;
          cf->b.s = 0;
          cf->w.s = XST_TLM;
          cf->w.t.tlm.spec = XSR_OP;
          cf->w.t.tlm.code = XSR_OP_I_INIT_REQUIRED;
          return ( 0 );
     }

     /* Key through */
     cf->w.d = X_F_RDC_TXE;
     while ( tcard.op != TX_COMMIT )
          xeris ( X_F_RDC );

     /* Clear the casefile for good measure */
     xeris ( X_S_CLR );
     return ( 0 );
}

     

     

     
