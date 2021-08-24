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
 *  XERIS CORE
 *
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  SIPLEX/STARLINE (AVR5)
 *
 *  Initilization Operator
 *
 */

#include <xeris.h>
#include <xeris/strap.h>
#include <std/tlm/siplex.h>
#include <core/flatline.h>
#include <core/std/dispatch.h>
#include <core/opsec.h>

#include <siplex/siplex.h>


static void init_lanes      ( xdcf * );
static void init_queues     ( xdcf * );
static void init_feeders    ( xdcf * );
static void init_primer     ( xdcf * );

static void abort_init      ( xdcf * cf, uint16_t soses );

static void banner          ( xdcf * );

uint16_t siplex_init ( xdcf * cf )
{
     
     /* Check cf and load opsec */
     if ( !cf->b.p )
     {
          abort_init ( cf, XSR_OP_F_CB_CONFIG );
          return ( 0 );
     }

     /* Global opcon init */
     opcon = (volatile struct opsec_opcon *)(cf->b.p);


     cf->b.p = NULL;
     cf->b.s = 0;
     /* Announce ourselves */

     banner ( cf );
     
     /* Hand-craft dispatch files from exec mem */
     init_lanes ( cf );

     /* Set-up mainline shot queues and initilize */
     /* magazine and shell pool */
     init_queues ( cf );

     /* Assign mainline feeders */
     init_feeders ( cf );

     /* Load primer shot into command queue */
     init_primer ( cf );

     /* Now for the moment of truth               */
     /* Make sure opcon is set-up with functional */
     /* defaults.                                 */
     opcon->p_soft = OPCON_OPSTRAP_P_SOFT;
     opcon->sslots = OPCON_OPSTRAP_SSLOTS;
     opcon->prothr = OPCON_OPSTRAP_PROTHR;
     opcon->dlmark = OPCON_OPSTRAP_DLMARK;
     opcon->dllimt = OPCON_OPSTRAP_DLLIMT;
     opcon->dlbump = OPCON_OPSTRAP_DLBUMP;

     /* Now configure the dispatch pointers */
     opcon->mission = &commissions[0];
     opcon->sched   = NULL;
     opcon->active  = NULL;

     /* Lastly, seed all lanes except lane zero, and then */
     /* do a cold start (exec_crank) on lane 1, since     */
     /* lane 1 is assigned the mainline command queue     */
     /* we can't do this ourself as the stack is going to */
     /* get shreded with us in it.                        */
     /* exec_boot () will work in the interrupt stack to  */

     /* we won't return from this... */
     /* goodbye. */
     exec_boot ( intzone.base );

     return ( 0 );
     
}


static void init_lanes ( xdcf * cf )
{
     uint16_t i, div;
     void * volatile exec_t, * volatile exec_b;

     /* Load in the executive memory extents         */
     /* These shall be set up in the active dispatch */
     /* zone file. Base = exec_b, Shock = exec_t     */
     exec_b = opcon->active->zone->base;
     exec_t = opcon->active->zone->shock;

     /* Now we need to divy-up the executive memory into */
     /* zones for our dispatch lanes, but first we       */
     /* reserve the interrupt stack                      */

     /* ISR stack comes out of bottom */
     intzone.base   = exec_b;

     /* claim the space */
     exec_b = (void *)( (ptr_t)exec_b -
                                 (SIPLEX_INTR_ZONE + SIPLEX_INTR_GUARD) );
     /* this actually pushes exec_b out one byte past our zone  */
     /* kind of like a stack, exec_b now points to the byte     */
     /* just above our zone, so we need to set the shock to one */
     /* byte below exec_b */
     /* Use increments because they are usually less work */
     intzone.shock = ++exec_b; 
     exec_b--;

     /* Set stack limit at below the guard zone */
     intzone.slim =
          (void *)((ptr_t)intzone.shock + (ptr_t)SIPLEX_INTR_GUARD);

     /* Put out the telemetry */
     cf->w.t.tlm.code = TLM_SIPLEX_CFG_INTZ;
     cf->w.t.tlm.dgm.dgm32 =
          ((volatile uint32_t)((volatile ptr_t)intzone.base)) -
          ((volatile uint32_t)((volatile ptr_t)intzone.shock));
     xeris ( 0 );
     
     /* Now we devide the boards according to the */
     /* configured number of lanes                */
     div =
     (uint16_t)( ((volatile ptr_t)exec_b -
                  (volatile ptr_t)exec_t) / SIPLEX_LANES );

     /* Report telemetry */
     cf->w.t.tlm.code = TLM_SIPLEX_CFG_ZONES;
     cf->w.t.tlm.dgm.dgm32 = (uint32_t)div;
     xeris ( 0 );

     
     /* Set up each lane */
     for ( i = 0; i < SIPLEX_LANES; i++ )
     {
          /* Config zones */
          zones[i].base  = exec_b;

          /* Claim space */
          exec_b = (void *)((ptr_t)exec_b - (ptr_t)div);
          /* like above, we need to set the shock at exec_b + 1 */
          zones[i].shock = ++exec_b;
          exec_b--;

          if ( exec_b < exec_t )
               abort_init ( cf, XSR_OP_F_INSUF_RESRC );

          zones[i].slim =
               (void *)( ((ptr_t)zones[i].shock) + SIPLEX_ZONE_BOARD );

          zones[i].thresh =
               (void *)(((ptr_t)zones[i].slim) + SIPLEX_ZONE_THRESH);
          zones[i].smark = zones[i].base;


          /* Config supervisor */
          supers[i].termret = 0;
          supers[i].termmrk = zones[i].base;

          /* Config Transaction Files */
          transacts[i].mark    = NULL;
          transacts[i].ctarget = 0;

          /* Fill-out dispatch file (commission) */
          commissions[i].status    = DS_OOS;
          commissions[i].priority  = 0;
          commissions[i].deadline  = 0xffff;
          commissions[i].contract  = sicon.def_contract;
          commissions[i].claim     = 0;
          commissions[i].cf        = NULL;
          commissions[i].zone      = &zones[i];
          commissions[i].super     = &supers[i];
          commissions[i].nx_binder = NULL;
          commissions[i].nx_sched  = NULL;
          commissions[i].transact  = &transacts[i];

          /* String commissions */
          if ( i > 0 )
               commissions[i-1].nx_binder = &commissions[i];

          /* Assign commissions */
          lanes[i].feed = NULL;
          lanes[i].recycle = &magazine;
          lanes[i].eject = &shells;
          lanes[i].commission = &commissions[i];
          
     }

     /* Report total lanes */
     cf->w.t.tlm.code = TLM_SIPLEX_CFG_LANES;
     cf->w.t.tlm.dgm.dgm32 = (uint32_t)i;
     xeris ( 0 );

     return;
}

/* Set-up shot queues */
static void init_queues ( xdcf * cf )
{
     register uint16_t i;
     
     /* Set-up queue affinities */
     /* A minimum functioning SIPLEX requires 4 "mainline" queues */
     /* Additional queues are marked as reserved                  */

     /* Initialize all queues */
     for ( i = 0; i < SIPLEX_SHOTQS; i++ )
     {
          queues[i].afn = Q_RSV;
          queues[i].head = NULL;
          queues[i].tail = NULL;
     }

     /* Report total queues */
     cf->w.t.tlm.code = TLM_SIPLEX_CFG_QUEUES;
     cf->w.t.tlm.dgm.dgm32 = (uint32_t)i;
     xeris ( 0 );

     /* Configure Mainline queues (first four) */
     queues[INIT_Q_CMD].afn = Q_CMD;     /* Command queue           */
     queues[INIT_Q_CYC].afn = Q_CYC;     /* Cyclical queue          */
     queues[INIT_Q_TRA].afn = Q_TRA;     /* Transaction queue       */
     queues[INIT_Q_INT].afn = Q_INT;     /* Interrupt Service queue */     
     
     /* Asign all available shots to a sabbot, and link   */
     /* all sabots. Sabots with a shot loaded are placed  */
     /* in the magazine (public shot pool), those without */
     /* are separated and attached to the shells pool     */
     /* (for private shots)  */

     for ( i = 0; i < (SIPLEX_SHOTS + SIPLEX_SHELLS); i++ )
     {
          if ( i > 0 )
               sabots[i-1].next = &sabots[i];

          if ( i < SIPLEX_SHOTS )
               shots[i].status = SHOT_FRE;

          sabots[i].next = NULL;
          sabots[i].shot = (i < SIPLEX_SHOTS) ? &shots[i] : NULL;
     }

     /* assign the empty head to our magazine           */
     /* and assign the reserved interrupt chain (if any) */
     /* to the interrupt queue (#3) */
     
     i = ( SIPLEX_SHOTS - 1 );
     magazine.head = &sabots[0];
     magazine.tail = &sabots[i];
     magazine.tail->next = NULL; /* cut-off at tail */


     /* Put the empty sabots in the shells pool    */
     /* Shots with the SHOT_PRV intent qualifier   */
     /* will be unloaded from the sabot, and the   */
     /* sabot will be linked into the shells queue */

     if ( SIPLEX_SHELLS > 0 )
     {
          shells.head = &sabots[SIPLEX_SHOTS];
          shells.tail = &sabots[(SIPLEX_SHOTS + SIPLEX_SHELLS) - 1];
     }
     else
     {
          shells.head = NULL;
          shells.tail = NULL;
     }

     return;
}

/* Set-up feeders */
static void init_feeders ( xdcf * cf )
{
     uint16_t i, this_depth, best_depth, next_free;
     register uint8_t k, q;

     volatile struct feedline * attach;

     /* First initialize the feedline pool so that all */
     /* Feedlines are all attached to NULL feeders     */
     for ( i = 0; i < SIPLEX_FEEDERS; i++ )
     {
          feeders[i].feeder = NULL;
          feeders[i].next   = NULL;
     }

     next_free = 0;
          
     /* the first 1-5 lines are initially configured in a     */
     /* default "primed" configuration, this is proceedurally */
     /* implemented as repeated runs over all available       */
     /* lanes until all objectives are satisfied,             */
     /* Always attaching the feeder to the least-             */
     /* encumbered lanes                                      */
     /* Objectives are as follows:                            */
     /* 1. 2 x INT   */
     /* 2. 1 x TRA   */
     /* 3. 1 x CMD   */
     /* 4. 1 x CYC   */
     
     /* A bare minimum of 2 dispatch lanes is recommened      */
     /* Such a configuration would result in the following    */
     /* Feeder config: */
     /* 0. INT->TRA->CYC */
     /* 1. INT->CMD */
     /* This bare minimum is required for multiprogramming    */
     /* However, the actual functional minimum is 1           */

     /* Any more than 5 lanes are left un-configured, and     */
     /* are therefore free for later configuration by the     */
     /* initial system */
     
     /* q = queue phase 0->3, CMD->CYC->TRA->INT              */
     /* i = lane select, k = objective,                       */

     for ( q = 0; q < 4; q++ )
     {
          /* Set-up the objective.                     */
          /* For now, only INT has an objective that's */
          /* not 1. So some hand-optimization here     */
          if ( q == INIT_Q_INT )
               k = 2;
          else
               k = 1;

          while ( k > 0 )
          {

               /* Find the least encombered lane */
               best_depth = 0xffff;
               
               for ( i = 0; i < SIPLEX_LANES; i++ )
               {
                    this_depth = 0;
                    attach = lanes[i].feed;

                    /* If this one is empty, we can just */
                    /* stop right here.                  */
                    if ( !attach )
                         break;

                    while ( attach->next )
                    {
                         this_depth++;
                         attach = attach->next;

                         if ( this_depth == 0xffff )
                              /* Very unlikely, but its */
                              /* not gonna happen.      */
                              break;
                    }

                    if ( this_depth < best_depth )
                         best_depth = this_depth;
               }

               /* Ok, so we either have a naked lane  */
               /* Or, we have a a tail feedline unit  */
               /* Depending on which, we need to add  */
               /* the next feeline unit and set it up */
               /* to point to our queue */

               if ( next_free >= SIPLEX_FEEDERS )
                    /* This means the configuration is */
                    /* insufficient, and is a big deal */
                    abort_init ( cf, XSR_OP_F_INSUF_RESRC );

               if ( attach )
               {
                    attach->next = &feeders[next_free++];
                    attach = attach->next;
               }
               else
               {
                    lanes[i].feed = &feeders[next_free++];
                    attach = lanes[i].feed;
               }

               /* Set up the feedline unit */
               attach->feeder = &queues[q];

               k--;

          }
     }
 
     return;
}

static void init_primer ( xdcf * cf )
{
     /* Simply load the primer into the mainline command  */
     /* queue, to call into STRAP%                        */

     volatile struct shotchain * ssabot;     /* Selected sabot  */

     
     /* Pop a free loaded sabot from the magazine  */
     ssabot = pop_shot ( &magazine );

     if ( !magazine.head )
          /* We don't have enough shots to load */
          /* the primer! */
          abort_init ( cf, XSR_OP_F_INSUF_RESRC );

     /* As per the XERIS/APEX specification, we are to */
     /* launch directly into STRAP% */
     ssabot->shot->master.b.p             = NULL;
     ssabot->shot->master.b.s             = 0;
     ssabot->shot->master.w.s             = XST_MSG;
     ssabot->shot->master.w.t.msg.ref     = 0;
     ssabot->shot->master.w.t.msg.arg.a64 = 0;

     /* Set specific settings */
     ssabot->shot->client     = X_SIPLEX;
     ssabot->shot->contractor = X_STRAP;
     ssabot->shot->intent     = SHOT_CMD;
     ssabot->shot->status     = SHOT_HOT;
     ssabot->shot->permit     = 0;
     
     /* Load the sabot on the command queue */
     put_shot ( &queues[INIT_Q_CMD], ssabot );

     return;

}

static void abort_init ( xdcf * cf, uint16_t soses )
{
     /* Fatal error on init, */
     /* return to FLATLINE% */
     cf->w.d = 0;
     cf->w.s = XST_RPT;
     cf->w.t.tlm.sec = X_SIPLEX;
     cf->w.t.tlm.spec = XSR_OP;
     cf->w.t.tlm.code = soses;

     xeris ( 0 );
     xeris ( XSEC_FLATLINE );
     return;
}


static void banner ( xdcf * cf )
{
     cf->w.s = XST_TLM;
     cf->w.t.tlm.spec = XSR_TLM_SIPLEX;
     cf->w.t.tlm.code = TLM_SIPLEX_CFG_PKG_STARLINE;
     cf->w.t.tlm.dgm.dgm32 = 0;
     xeris ( 0 );

     cf->w.t.tlm.code = TLM_SIPLEX_CFG_ARCH;
     xeris ( 0 );

     cf->w.t.tlm.code = TLM_SIPLEX_CFG_DEV;
     xeris ( 0 );

     cf->w.t.tlm.code = TLM_SIPLEX_CFG_VMAG;
     cf->w.t.tlm.dgm.dgm32 = 1;
     xeris ( 0 );

     cf->w.t.tlm.code = TLM_SIPLEX_CFG_VMIN;
     cf->w.t.tlm.dgm.dgm32 = 0;
     xeris ( 0 );

     return;
}

          
     


     

     
          
          


     
