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
 *  tx.c
 *  Transaction Handlers
 */

#include <xeris.h>
#include <xeris/f/rdc.h>

#include <f/rdc/rdc.h>

static void prep_event
( register struct tx_ledger ** current,
  register struct tx_ledger ** event );

uint16_t txp ( xdcf * cf )
{
     /* Push Transaction. This desk gets called when rdc% */
     /* is already in a transaction. */

     struct tx_card * card;
     struct tx_ledger * current, * event;

     if ( (!cf->b.p) || (cf->b.s != sizeof ( struct tx_card )) )
     {
          cf->b.p = NULL;
          cf->b.s = 0;
          /* This should be impossible, but never assume! */
          return ( 0 );
     }

     card = (struct tx_card *)cf->b.p;

     if ( (!tx_account.current) || (!tx_account.event) )
     {
          if ( card->op == TX_INIT1 )
          {
               /* Called from init() to test the init    */
               /* state. If we have done init1, we won't */
               /* get here, blocking other inits  */

               tx_account.event = (void *)&lega;
               return ( 0 );
          }

          if ( card->op == TX_INIT2 )
          {
               /* Last init and we should be good! */
               tx_account.event = (void *)&legb;
               return ( 0 );
          }

          /* If the operation is not TX_INIT, then it */
          /* means we just arn't ready. */
          cf->b.p          = NULL;
          cf->b.s          = 0;
          cf->w.s          = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_I_INIT_REQUIRED;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     if ( card->op >= TX_INIT1 )
     {
          /* We are already inited, so this is "illegal" */
          cf->b.p          = NULL;
          cf->b.s          = 0;
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_I_INIT_ILLEGAL;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }


     /* Everything checks-out, so we can try to push the */
     /* transaction card to the queue */
     
     /* Set-up pointers and copy current to event */
     prep_event ( &current, &event );
     
     card->next = NULL;

     if ( (!event->e_last) || (!event->e_first) )
     {
          event->e_first = card;
          event->e_last  = card;
     }
     else
     {
          event->e_last->next = card;
          event->e_last = card;
     }

     /* it's THAT easy. */
     return ( 0 );
}
          

uint16_t txe ( void )
{
     /* Exec Transaction. This desk gets called when rdc% */
     /* is already in a transaction. */

     struct tx_card * card;
     struct tx_ledger * current, * event;
     struct dispatch_q * tq;
     uint8_t op;

     /* Set up our event account and pointers */
     prep_event ( &current, &event );

     /* try to get the "frist" card */
     card = current->e_first;

     if ( !card )
     {
          /* No more transactions! */
          event->e_first = NULL;
          event->e_last  = NULL;
          return ( 0 );
     }

     if ( current->e_first == current->e_last )
     {
          /* This is the last card */
          event->e_first = NULL;
          event->e_last  = NULL;
     }
     else
     {
          /* Advance the queue */
          event->e_first = card->next;
          event->e_last  = current->e_last;
     }

     /* Now we can process the actual transaction cards */
     /* The actual operation is the low nibble */
     op = ( card->op & 0x0f );

      /* Set up the queue pointer to the appropriate queue */
     if ( (card->op & 0xf0) == TX_DISPATCH )
          tq = &event->d_dispatch;
     else
          tq = &event->d_free;

     /* Finally do the actual thing */
     if ( op == TX_POP )
     {
          card->dispatch = tq->first;
          
          if ( (tq->first == tq->last) || (!tq->first) )
          {
               /* last one! or no one! */
               tq->first = NULL;
               tq->last  = NULL;
          }
          else
          {
               tq->first = tq->first->next;
          }

     }

     if ( op == TX_PUSH )
     {
          card->dispatch->next = NULL;
          
          if ( (!tq->first) || (!tq->last) )
          {
               /* tq-> first and last should ALWAYS be  */
               /* null together, unless someone else is */
               /* messing with us.. which could happen  */
               /* this way we might end-up exhausting   */
               /* dispatch cards, but it wont go crazy  */
               tq->first = card->dispatch;
               tq->last  = tq->first;
          }
          else
          {
               tq->last->next = card->dispatch;
               tq->last = card->dispatch;
          }

     }
     

     card->op = TX_COMMIT;
     return ( 0 );          
}
          
static void prep_event
( register struct tx_ledger ** current,
  register struct tx_ledger ** event )
{
     *current = (struct tx_ledger *)tx_account.current;
     *event   = (struct tx_ledger *)tx_account.event;

     **event = **current;
     return;
}
