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
 *  XERIS Logistics Group
 *
 *  l/track%
 *  Track Registrar
 *
 *  GENERIC Release 1
 *  Version 1.0
 *
 *  op_tx.c
 *  Transaction Operator
 *
 */

#include <xeris.h>

#include "track.h"

static void op_tx_push ( xdcf * );
static void op_tx_exec ( xdcf * );

uint16_t op_tx ( xdcf * cf )
{
     /* Basically, we're routing for only two "desks" */

     if ( cf->w.d == TX_PUSH )
          op_tx_push ( cf );
     else
          op_tx_exec ( cf );

     /* Since we are in the XERIS Transaction, we must */
     /* return zero! */

     return ( 0 );
}

static void op_tx_push ( xdcf * cf )
{
     /* Push a transaction card onto the queue         */
     /* Note, by nature, each transaction card is      */
     /* submitted by our reception desk, via a         */
     /* single dispatch. This means only one dispatch  */
     /* can generate a transaction card for this queue */
     /* Therefore the card is on the stack for the     */
     /* dispatch lane */

     struct txaccnt * current, * event;
     struct txcard * card;

     current = (struct txaccnt *)transact.current;
     event   = (struct txaccnt *)transact.event;
     card    = (struct txcard  *)cf->b.p;

     /* Add new card to the back */

     /* Since the card at the back should ALWAYS */
     /* point to NULL (this is enforced by tx_   */
     /* exec). Modifying q_last's "next" will    */
     /* either be successful in this transaction */
     /* or it will be caught and corrected by    */
     /* tx_exec, in the "worst case" */
     card->next = NULL;
     if ( current->q_last )
          current->q_last->next = card;

     /* Here's the real "transaction" magic */
     event->q_last = card;
     event->q_first =
          current->q_first ? current->q_first : card;

     /* We need to maintain the entire account! */
     event->freelist = current->freelist;

     /* "Commit" */
     return;
}


static void op_tx_exec ( xdcf * cf )
{
     /* We don't know how many times we "redo" this part  */
     /* The point of this set-up is that its *always* the */
     /* card at the front of the queue that gets executed */
     /* And the queue only gets shorter if the card is    */
     /* executed in it's entirty (a transaction by        */
     /* definition) */

     struct txaccnt * current, * event;
     struct txcard * card;

     uint8_t idx;

     current = (struct txaccnt *)transact.current;
     event   = (struct txaccnt *)transact.event;

     /* Ensure no errors in the transition */
     event->q_last = current->q_last;
     event->freelist = current->freelist;

     /* Load in the front card */
     card    = current->q_first;
     idx = card->session.idx;

     if ( !card )
     {
          /* No more transactions! */
          event->q_first = NULL;
          event->q_last  = NULL;
          return;
     }

     if ( current->q_first == current->q_last )
     {
          /* this is the last card */
          event->q_first = NULL;
          event->q_last  = NULL;
     }
     else
     {
          /* Advance queue */
          event->q_first = card->next;
          event->q_last  = current->q_last;
     }

     /* Go to town! */

     switch ( card->op )
     {
         case TX_NEW:
              /* Attempt to reserve a new Track */
              /* Registration file */
              if ( !current->freelist )
              {
                   card->result = TX_FAIL;
                   break;
              }
              
              event->freelist = current->freelist->next;
              card->session.idx = current->freelist->idx;
              card->result = TX_OK;
              break;

         case TX_TERM:
              /* Cancel a track reservation */
              tracks[idx].next =
                   current->freelist;
              event->freelist = &tracks[idx];
              card->result = TX_OK;
              break;

         case TX_UPDATE:
              /* Again, due to the nature of our queue, */
              /* even if this gets interrupted, it's    */
              /* going to get re-done before any other  */
              /* so we don't need to worry about        */
              /* incomplete writes. */
              tracks[idx].next     = NULL;
              tracks[idx].dispatch = card->session.dispatch;
              tracks[idx].adm.sid  = card->session.adm.sid;
              tracks[idx].adm.dsk  = card->session.adm.dsk;
              tracks[idx].agt.sid  = card->session.agt.sid;
              tracks[idx].agt.dsk  = card->session.agt.dsk;

              card->result = TX_OK;
              break;

         case TX_INIT:
              /* Is what it looks like */

              for ( register uint8_t i = 0; i < TTRACKS; i++ )
              {
                   tracks[i].idx      = i;
                   tracks[i].next     = NULL;
                   tracks[i].dispatch = 0;
                   tracks[i].adm.sid  = 0;
                   tracks[i].adm.dsk  = 0;
                   tracks[i].agt.sid  = 0;
                   tracks[i].agt.dsk  = 0;
              }

              is_init      = 0xff;
              card->result = TX_OK;
              break;

         case TX_PULL:
         default:
              /* Pull the file at the specified */
              /* Index */
               
              card->session.next     = NULL;
              card->session.dispatch = tracks[idx].dispatch;
              card->session.adm.sid  = tracks[idx].adm.sid;
              card->session.adm.dsk  = tracks[idx].adm.dsk;
              card->session.agt.sid  = tracks[idx].adm.sid;
              card->session.agt.dsk  = tracks[idx].adm.dsk;

              card->result = TX_OK;
              break;

     }

     /* Commit! */
     return;
}     



     

     
     

     





          
               
               
               
