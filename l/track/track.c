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
 *  track.c
 *  Dispatch Desk
 *
 */

#include <xeris.h>

#include "track.h"

/* Define our globals */
struct txaccnt tx_b;
struct txaccnt tx_a =
{
     .q_first  = NULL,
     .q_last   = NULL,
     .freelist = NULL
};

struct trf tracks[TTRACKS];

uint8_t is_init = 0;

xta transact =
{
     .current = (void *)&tx_a,
     .event   = (void *)&tx_b,

     .act     = 0,
     .com     = 0
};

static uint16_t reception_desk ( xdcf * );

static void     pull_trf    ( xdcf * );
static void     lea_trf     ( xdcf * );
static void     do_adm      ( xdcf * );

static void     norec_tic   ( xdcf * );
static void     admok_tic   ( xdcf * );

/* Dispatch Desk */
uint16_t main ( xdcf * cf )
{
     
     /* Check if we're entering a transaction */
     /* Don't waste stack space */
     if ( cf->s.sid == TRANSACTION )
          return op_tx ( cf );

     return reception_desk ( cf );
}

static uint16_t reception_desk ( xdcf * cf )
{
     struct txcard tx;
     
     /* Check for init */
     if ( is_init != 0xff )
     {
          /* Not initialized. Do that first. */

          /* Get init transaction card loaded */
          cf->w.d = TX_PUSH;
          tx.op = TX_INIT;
          xeris ( TRANSACTION );

          /* Key it through */
          cf->w.d = TX_EXEC;
          while ( tx.op != TX_OK )
               xeris ( TRANSACTION );

     }

     /* Screen tickets first */
     if
     (
          ( cf->w.s != XST_TRAK ) &&
          ( cf->w.s != XST_TLEA ) &&
          ( cf->w.s != XST_TADM )
     )
     {
          /* Reject! */
          cf->w.s = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     cf->b.p = (void *)&tx;
     cf->b.s = sizeof ( struct txcard );
     
     /* OK, now we can get down to our business       */
     /* All operations except a lease request require */
     /* us to pull a specific TRF. So let's first see */
     /* if it's a lease request, then take it from    */
     /* there. */

     if ( cf->w.s == XST_TLEA )
     {
          lea_trf ( cf );
          return ( 0 );
     }

     /* We know we need to pull the TRF first     */
     /* If we can't find it, then its a NO_RECORD */

     /* Check index range */
     /* for all track tickets except lease tickets,   */
     /* the first feild is always a uint32_t track ID */
     if ( cf->w.t.trak.track > TTRACKS )
     {
          norec_tic ( cf );
          return ( 0 );
     }

     cf->w.d = TX_PUSH;
     tx.op   = TX_PULL;
     tx.session.idx = (uint8_t)cf->w.t.trak.track;
     xeris ( TRANSACTION );

     /* Key-through */
     cf->w.d = TX_EXEC;
     while ( tx.result != TX_OK )
          xeris ( TRANSACTION );

     /* If the registered admin SID is 0, that means */
     /* the file is not leased, i.e. no record.      */
     if ( !tx.session.adm.sid )
     {
          norec_tic ( cf );
          return ( 0 );
     }
     
     if ( cf->w.s == XST_TRAK )
     {
          /* Process the query request  */
          pull_trf ( cf );
          return ( 0 );
     }
          
     /* Must be an admin ticket */
     do_adm ( cf );
     return ( 0 );
}

static void pull_trf ( xdcf * cf )
{
     struct txcard * tx;

     tx = (struct txcard *)cf->b.p;
     cf->w.s = XST_TRAK;
     cf->w.t.trak.track = (uint32_t)tx->session.idx;
     cf->w.t.trak.dispatch = tx->session.dispatch;
     
     if ( cf->s.sid == tx->session.adm.sid )
     {
          /* Fill in agent info */
          if ( !tx->session.agt.sid )
          {
               /* Track dropped! */
               cf->w.s = XST_DNY;
               cf->w.t.rpt.spec = XSR_OP;
               cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
               cf->w.t.rpt.cond = RPT_CNC;
               cf->w.t.rpt.note = RPT_NON;
          }
          else
          {
               cf->w.t.trak.sid = tx->session.agt.sid;
               cf->w.t.trak.dsk = tx->session.agt.dsk;
          }
     }
     else if ( cf->s.sid == tx->session.agt.sid )
     {
          /* Fill in admin info */
          cf->w.t.trak.sid = tx->session.adm.sid;
          cf->w.t.trak.dsk = tx->session.adm.dsk;
     }
     else
     {
          /* Requesting secretariat is not party */
          /* to the lease. */
          norec_tic ( cf );
     }

     return;
}

          

static void lea_trf ( xdcf * cf )
{
     /* For track lease operations, the basic procession */
     /* is as follows:                                   */
     /* 1. Attept to obtain a free lease file            */
     /* 3. Update obtained lease file with correct       */
     /*    information.                                  */

     struct txcard *tx;

     tx = (struct txcard *)cf->b.p;

     /* Attempt new */
     cf->w.d = TX_PUSH;
     tx->op  = TX_NEW;
     xeris ( TRANSACTION );

     cf->w.d = TX_EXEC;
     while ( tx->op == TX_NEW )
          xeris ( TRANSACTION );

     if ( tx->op == TX_FAIL )
     {
          /* Sorry */
          cf->b.p = NULL;
          cf->b.s = 0;

          cf->w.s = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return;
     }

     /* Otherwise, it was successful. Set-up update */
     tx->session.dispatch = cf->w.t.tlea.dispatch;
     tx->session.adm.sid  = cf->s.sid;
     tx->session.adm.dsk  = cf->w.t.tlea.admin.dsk;
     tx->session.agt.sid  = cf->w.t.tlea.agent.sid;
     tx->session.agt.dsk  = cf->w.t.tlea.agent.dsk;

     cf->w.d = TX_PUSH;
     tx->op  = TX_UPDATE;
     xeris ( TRANSACTION );
     
     /* Key it through */
     cf->w.d = TX_EXEC;
     while ( tx->result != TX_OK )
          xeris ( TRANSACTION );

     /* Done. */
     cf->b.p = NULL;
     cf->b.s = 0;

     cf->w.s = XST_TRAK;
     cf->w.t.trak.track    = (uint32_t)tx->session.idx;
     cf->w.t.trak.dispatch = tx->session.dispatch;
     cf->w.t.trak.sid      = tx->session.agt.sid;
     cf->w.t.trak.dsk      = tx->session.agt.dsk;

     return;
}

static void do_adm ( xdcf * cf )
{
     struct txcard * tx;

     tx = (struct txcard *)cf->b.p;

     if
     (
          ( cf->s.sid != tx->session.adm.sid ) &&
          ( cf->s.sid != tx->session.agt.sid )
     )
     {
          /* Submitting secretariat is not party to this lease */
          norec_tic ( cf );
          return;
     }

     if ( cf->w.t.tadm.op == TRAK_TRM )
     {
          if ( cf->s.sid == tx->session.adm.sid )
          {
               /* Full termination goin down */
               cf->w.d = TX_PUSH;
               tx->op  = TX_TERM;
               xeris ( TRANSACTION );

               cf->w.d = TX_EXEC;
               while ( tx->op != TX_OK )
                    xeris ( TRANSACTION );

               admok_tic ( cf );
               return;
          }
          else
          {
               /* Track drop */
               tx->session.agt.sid = 0;
               tx->session.agt.dsk = 0;
          }
     }
     else
     {
          if ( cf->s.sid == tx->session.agt.sid )
          {
               /* Not autorized */
               cf->w.s = XST_DNY;
               cf->w.t.rpt.spec = XSR_OP;
               cf->w.t.rpt.code = XSR_OP_F_NO_AUTH;
               cf->w.t.rpt.cond = RPT_CNC;
               cf->w.t.rpt.note = RPT_NON;
               return;
          }

          switch ( cf->w.t.tadm.op )
          {
               case TRAK_XFR:
                    /* Transfer lease */
                    /* We will use the ticket .reg space */
                    /* as our scratch pad */

                    if ( !tx->session.agt.sid )
                    {
                         /* No agent registered! we can't */
                         /* continue */
                         cf->b.p = NULL;
                         cf->b.s = 0;
                         cf->w.s = XST_UNA;
                         cf->w.t.rpt.spec = XSR_OP;
                         cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
                         cf->w.t.rpt.cond = RPT_CNC;
                         cf->w.t.rpt.note = RPT_NON;
                         
                         return;
                    }
                    
                    cf->w.t.tadm.com.reg.sid = tx->session.adm.sid;
                    cf->w.t.tadm.com.reg.dsk = tx->session.adm.dsk;
                    tx->session.adm.sid      = tx->session.agt.sid;
                    tx->session.adm.dsk      = tx->session.agt.dsk;
                    tx->session.agt.sid      = cf->w.t.tadm.com.reg.sid;
                    tx->session.agt.dsk      = cf->w.t.tadm.com.reg.dsk;
                    
                    break;

               case TRAK_RED:
                    /* Redirect (update agent) */
                    tx->session.agt.sid = cf->w.t.tadm.com.reg.sid;
                    tx->session.agt.dsk = cf->w.t.tadm.com.reg.dsk;
                    
                    break;

               case TRAK_DSP:
                    /* Update dispatch ID */
                    tx->session.dispatch = cf->w.t.tadm.com.dispatch;

                    break;

               default:
                    /* Invalid option */
                    cf->b.p = NULL;
                    cf->b.s = 0;
                    cf->w.s = XST_RJT;
                    cf->w.t.rpt.spec = XSR_OP;
                    cf->w.t.rpt.code = XSR_OP_F_OPTION_UNKN;
                    cf->w.t.rpt.cond = RPT_FTL;
                    cf->w.t.rpt.note = RPT_NON;
                    
                    return;
                    break;
          }
          
     }

     /* Execute any update */
     cf->w.d = TX_PUSH;
     tx->op  = TX_UPDATE;
     xeris ( TRANSACTION );
     
     cf->w.d = TX_EXEC;
     while ( tx->op != TX_OK )
          xeris ( TRANSACTION );

     admok_tic ( cf );
     
     return;
}

          

static void norec_tic ( xdcf * cf )
{
     cf->b.p = NULL;
     cf->b.s = 0;

     cf->w.s = XST_UNA;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_NO_RECORD;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;

     return;
}


static void admok_tic ( xdcf * cf )
{
     cf->b.p = NULL;
     cf->b.s = 0;

     cf->w.s          = XST_ACP;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_ACCEPTED;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_NON;

     cf->w.t.rpt.adi.adi32 = 0;

     return;
}    
