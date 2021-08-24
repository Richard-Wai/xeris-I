/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All rights reserved.
 */

/*
 *  XERIS CORE
 *
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  SIPLEX/STARLINE (AVR5)
 *
 *  Shot Submission Desk
 *  SUB$
 *
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <core/atria.h>
#include <core/opsec.h>

#include <siplex/siplex.h>

/* Assistants */
static uint16_t shotlease ( xdcf * cf );
static uint16_t shotpriv  ( xdcf * cf );
static uint16_t shotcanc  ( xdcf * cf );

uint16_t subdesk ( xdcf * cf )
{
     /* Route the Case File to proper assistant */

     switch ( cf->w.t.srq.rqc )
     {
          case X_SIPLEX_SUB_LEAS:
               return ( shotlease ( cf ) );
               break;

          case X_SIPLEX_SUB_PRIV:
               return ( shotpriv ( cf ) );
               break;

          case X_SIPLEX_SUB_CANC:
               return ( shotcanc ( cf ) );
               break;

          default:
               break;
     }


     /* No service */
     cf->w.s          = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );

}

static uint16_t shotlease ( xdcf * cf )
{

     volatile struct shotchain * sabot;
     volatile struct shotqueue * q;
     uint8_t                     intent;
     
     /* Check security level */
     if ( cf->s.sec < sicon.lvl_lease )
     {
          /* Sorry */
          cf->w.s          = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Check the board */
     if ( (!cf->b.p) || (cf->b.s != sizeof (xdcf)) )
     {
          /* Invalid board config */
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_CB_CONFIG;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     
     /* Screen intent */
     intent = (uint8_t)(cf->w.t.srq.prm.p16[1] & 0x00ff);

     if ( intent > SHOT_PER )
     {
          /* Invalid board config */
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_PARAM_EXCUR;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Obtain queue route */
     q = siplex_route ( intent, cf->w.t.srq.prm.p16[2] );

     if ( !q )
     {
          /* Invalid board config */
          cf->w.s          = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_NOT_PERMITTED;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }
     

     /* OK, finally, lets try to get a free shot from */
     /* the magazine (loaded sabot)                   */
     sabot = pop_shot ( &magazine );

     if ( !sabot )
     {
          /* No free sabots, sorry */
          cf->w.s          = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }


     /* Great, we got a sabot, and we have a queue  */
     /* Everything is looking good. Load and prime */
     /* the shot */

     sabot->shot->status     = SHOT_HOT;
     sabot->shot->intent     = intent;
     sabot->shot->client     = cf->s.sid;
     sabot->shot->contractor = cf->w.t.srq.prm.p16[0];
     sabot->shot->permit     = cf->w.t.srq.prm.p16[2];

     /* Copy case file */
     mem_copy
     ( cf->b.p, (void *)(&sabot->shot->master), sizeof (xdcf) );

     /* Lastly, we now load the sabot into the queue */
     put_shot ( q, sabot );

     cf->w.s          = XST_ACP;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_ACCEPTED;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}

static uint16_t shotpriv ( xdcf * cf )
{

     volatile        shotcard  * shot;
     volatile struct shotchain * sabot;
     volatile struct shotqueue * q;
     
     /* Check security level */
     if ( cf->s.sec < sicon.lvl_priv )
     {
          /* Sorry */
          cf->w.s          = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Check the board */

     shot = (volatile shotcard *)cf->b.p;
     
     if ( (!shot) || (cf->b.s != sizeof (shotcard)) )
     {
          /* Invalid board config */
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_CB_CONFIG;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     
     /* Obtain queue route */
     q = siplex_route ( shot->intent, shot->permit  );

     if ( !q )
     {
          /* Invalid board config */
          cf->w.s          = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_NOT_PERMITTED;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }
     

     /* OK, finally, lets try to get a free shot from */
     /* the magazine (loaded sabot)                   */
     sabot = pop_shot ( &magazine );

     if ( !sabot )
     {
          /* No free sabots, sorry */
          cf->w.s          = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }


     /* Great, we got a sabot, and we have a queue  */
     /* Everything is lookign good. Load and prime */
     /* the shot */

     shot->status     = SHOT_HOT;
     shot->client     = cf->s.sid;

     /* Load shot into sabot */
     sabot->shot = shot;


     /* Lastly, we now load the sabot into the queue */
     put_shot ( q, sabot );

     cf->w.s          = XST_ACP;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_ACCEPTED;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}

static uint16_t shotcanc ( xdcf * cf )
{
     cf->w.s          = XST_DNY;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}
