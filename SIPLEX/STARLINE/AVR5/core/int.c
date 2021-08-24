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
 *  Interrupts Supervisor
 *
 */

#include <xeris.h>
#include <core/atria.h>
#include <core/siplex.h>

#include <siplex/siplex.h>
#include <siplex/int.h>

const __memx struct atria_sisr * volatile int_sisr;

uint16_t siplex_int_init ( xdcf * cf )
{

     volatile struct shotchain * sabot;
     
     /* Initialize interrupts and enable           */
     /* Insert supervisor ticket into cyclic queue */

     /* First, we check if this has been done already */
     /* if so, we exit with an error */

     if ( opcon->intr->sisr )
     {
          cf->w.s = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_PROC_VIOL;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }


     /* Make sure interrupts are disabled */
     int_dis ( );

     /* Configure interrupt stack */
     opcon->intr->sbase = intzone.base;
     opcon->intr->slim  = intzone.slim;
     opcon->intr->smark = intzone.smark;

     /* clear message queues */
     for ( register uint16_t i = 0; i < SIPLEX_INTR_QUEUE; i++ )
     {
          intmsg[0][i].irq = 0;
          intmsg[1][i].irq = 0;
          intmsg[0][i].msg = 0;
          intmsg[1][i].msg = 0;
     }

     /* assign queue */
     opcon->intr->msg_q   = &intmsg[0][0];
     opcon->intr->msg_max = &intmsg[0][SIPLEX_INTR_QUEUE - 1];

     /* Interrupt config settings are handled by */
     /* the control operator (siplex_int_ctrl())*/

     /* Ensure soft preemption is off */
     opcon->p_soft = 0;

     /* Finally, retreive the SISR root from ATRIA */
     siplex_int_sisr ( cf );


     /* Attempt to register supervisor */
     sabot = pop_shot ( &magazine );

     if ( !sabot )
     {
          /* No available  */
          cf->w.s = XST_UNA;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Configure the shot */
     sabot->shot->status     = SHOT_HOT;
     sabot->shot->intent     = SHOT_CYC;
     sabot->shot->client     = X_SIPLEX;
     sabot->shot->contractor = X_SIPLEX;
     sabot->shot->permit     = 0;

     /* Configure Case File */
     sabot->shot->master.w.d             = X_SIPLEX_SYS;
     sabot->shot->master.w.s             = XST_SRQ;
     sabot->shot->master.w.t.srq.rqc     = X_SIPLEX_SYS_INT_SUPR;
     sabot->shot->master.w.t.srq.prm.p64 = 0;

     /* put shot on mainline cyclic queue */
     put_shot ( &queues[INIT_Q_CYC], sabot );

     /* Finally, we will set all-kill and disable nesting */
     opcon->intr->kilmsk = 0xff;
     opcon->intr->throtl = 0x00;

     /* turn on interrupts! (perminently) */
     int_ena ( );

     /* Done */
     cf->w.s = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_I_INIT_OK;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );

}

uint16_t siplex_int_kill ( xdcf * cf )
{
     register uint8_t tmp_throtl, tmp_kilmsk;
     
     /* This service sets the interrupt throtle and kill masks */
     /* it returns the previous values. */
     /* prm08[0] - throtl */
     /* prm08[1] - kilmsk */

     /* A report is returned on completion with the current */
     /* values */

     tmp_throtl =
          a8 ( (volatile void *)&opcon->intr->throtl, cf->w.t.srq.prm.p08[0] );
     tmp_kilmsk =
          a8 ( (volatile void *)&opcon->intr->kilmsk, cf->w.t.srq.prm.p08[1] );
     
     cf->w.s = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_RET;

     cf->w.t.rpt.adi.adi08[0] = tmp_throtl;
     cf->w.t.rpt.adi.adi08[1] = tmp_kilmsk;
     return ( 0 );
}

uint16_t siplex_int_timr ( xdcf * cf )
{
     register uint16_t tmp_dtimer;

     /* This service sets the designated timer which */
     /* is authorized to trigger preemptions         */
     /* This service returns the previous value.     */
     /* prm16[0] - dtimer */

     tmp_dtimer =
          a8 ( (volatile void *)&opcon->intr->dtimer, cf->w.t.srq.prm.p16[0] );

     cf->w.s = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_RET;

     cf->w.t.rpt.adi.adi16[0] = tmp_dtimer;
     return ( 0 );
}

uint16_t siplex_int_sisr ( xdcf * cf )
{
     register uint8_t tkill;
     
     cf->w.d = OP_ATRIA_INTR;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = SVC_ATRIA_INTR_REFISRT;
     xeris ( XSEC_ATRIA );

     if ( cf->w.s != XST_MSG )
     {
          if ( cf->w.s == XST_EXC )
               return ( 0 );

          if ( cf->w.s != XST_RPT )
          {
               cf->w.t.tlm.spec = XSR_OP;
               cf->w.t.tlm.code = XSR_OP_F_UNDEFINED;
          }
          
          /* Didn't work... raise exception */
          cf->w.s = XST_EXC;
          cf->w.t.tlm.sec  = XSEC_ATRIA;
          return ( 0 );
     }
     /* Cool, so for our platform, this must be a */
     /* "memx" pointer */
     int_sisr = (const __memx struct atria_sisr *)cf->w.t.msg.arg.a64;
     
     /* Temporarily shut-down all interrupts (!!) */
     tkill = a8 ( (volatile void *)&opcon->intr->kilmsk, 0xff );
     
     /* Load reference into OPSEC */
     opcon->intr->sisr = cf->w.t.msg.arg.a64;

     /* Restore kill flags */
     a8 ( (volatile void *)&opcon->intr->kilmsk, tkill );

     cf->w.s = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_OPR_OK;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );

}


uint16_t siplex_int_supr ( xdcf * cf )
{
     /* Check the interrupt message queue  */
     /* and attempt to dispatch messages   */

     volatile struct siplex_imsg * outq, * inq;
     volatile struct shotchain * sabot;

     /* identify active outq (outq) */
     /* switch interrupt system to new queue (inq) */
     outq = opcon->intr->msg_max;
     
     if ( outq == &intmsg[0][SIPLEX_INTR_QUEUE - 1] )
     {
          outq = &intmsg[0][0];
          inq  = &intmsg[1][0];
               
     }
     else if ( outq == &intmsg[1][SIPLEX_INTR_QUEUE - 1] )
     {
          /* make no assumptions */
          outq = &intmsg[1][0];
          inq  = &intmsg[0][0];
     }
     else
     {
          /* This is bullshit. */
          /* Flush the whole thing and reset everything. */
          int_swmq
          (
               &opcon->intr->msg_q,   &intmsg[0][0],
               &opcon->intr->msg_max, &intmsg[0][SIPLEX_INTR_QUEUE - 1]
          );
          
          return ( 0 );
     }

     /* First, check to see if queue actually */
     /* has any messages */
     if ( !outq[0].msg )
          /* no messages */
          return ( 0 );
     
     /* switch queue */
     int_swmq
     (
          &opcon->intr->msg_q,   &inq[0],
          &opcon->intr->msg_max, &inq[SIPLEX_INTR_QUEUE - 1]
     );


     /* Now we can get to work */
     /* Starting at the top, we try make a shot */
     /* for each message in the queue */

     for ( register uint16_t i = 0; i < SIPLEX_INTR_QUEUE; i++ )
     {
          if ( !outq[i].msg )
               break;

          /* attempt to get a free loaded sabot */
          sabot = pop_shot ( &magazine );

          while ( !sabot )
          {
               register uint8_t popcon;
               
               /* Couldn't get one, so we need to  */
               /* adjust our dispatch contract and */
               /* priority, then keep trying until */
               /* we get one.                      */

               /* we will pose as an executor so   */
               /* that opsec keeps scheduling us   */
               /* every cycle */

               popcon = opcon->active->contract;
               opcon->active->contract = 1;
               opcon->active->status   = DS_EXEC_IDLE;
               xeris ( 0 );

               sabot = pop_shot ( &magazine );

               if ( sabot )
               {
                    /* great we got one */
                    opcon->active->status = DS_EXE;
                    opcon->active->contract = popcon;
                    break;
               }
          }

          /* set up shotcard */
          sabot->shot->status = SHOT_HOT;
          sabot->shot->intent = SHOT_CMD;
          sabot->shot->client = int_sisr[outq[i].irq].office;
          sabot->shot->contractor = sabot->shot->client;

          /* set up Case File */
          sabot->shot->master.w.d                = X_STD_ISR;
          sabot->shot->master.w.t.msg.ref        = outq[i].irq;
          sabot->shot->master.w.t.msg.arg.a08[0] = outq[i].msg;

          /* Put on the interrupt service queue */
          put_shot ( &queues[INIT_Q_INT], sabot );

          /* Clear the message from the queue */
          outq[i].irq = 0;
          outq[i].msg = 0;

     }

     return ( 0 );
}




     
          
          
                    
                    
               

               
               


     
        




     

     

     

     


     
     



     
