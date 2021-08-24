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
 *  ctl.c
 *  ctl$ desk
 */

#include <xeris.h>
#include <xeris/siplex.h>
#include <xeris/f/rdc.h>

#include <f/rdc/rdc.h>

uint16_t ctl ( xdcf * cf )
{
     /* From the designated controller? */
     if ( cf->s.sid != dctrl )
     {
          cf->w.s          = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }
     
     /* Is it a service request ticket? */
     if ( cf->w.s != XST_SRQ )
     {
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Finally check the actual request code */

     if ( cf->w.t.srq.rqc == X_F_RDC_CTL_AGENT )
          /* Try to register a new agent! */
          return reg ( cf );
     
     if ( cf->w.t.srq.rqc == X_F_RDC_CTL_CTRL )
     {
          /* We're not going to go through all the   */
          /* hassle of putting this through a        */
          /* transaction queue. It should be really  */
          /* infrequeuent, and it really should not  */
          /* possibly be something that gets run     */
          /* through multiple lanes at once */
          
          dctrl = cf->w.t.srq.prm.p16[0];
          return ( 0 );
     }
          
     if ( cf->w.t.srq.rqc ==  X_F_RDC_CTL_INIT )
          return init ( cf );
                    
     /* SVC is no good */
     cf->w.s = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}


uint16_t init ( xdcf * cf )
{
     /* The init thing! */
     volatile struct tx_card card;
     register uint16_t i;

     /* First we will attempt to "push" */
     /* an TX_INIT. */
     card.op = TX_INIT1;     
     cf->b.p    = (void *)&card;
     cf->b.s    = sizeof ( struct tx_card );
     cf->w.d    = X_F_RDC_TXP;
     xeris ( X_F_RDC );

     /* If we are already initialized, the Board Pointer */
     /* will be nullified by txp$, which will also set   */
     /* up the report ticket for us. */
     if ( !cf->b.p )
          return ( 0 );


     /* If we got here, it means we have done "phase 1" */
     /* so we are now exclusive to this part, and the   */
     /* secretariat over-all still considers itself un- */
     /* initialized */

     /* We really just need to set-up the queues */
     ds_stack[DS_STACK_SIZE - 1].next = NULL;
     for ( i = 1; i < DS_STACK_SIZE; i++ )
          ds_stack[i-1].next = &ds_stack[i];

     legb.e_first          = NULL;
     legb.e_last           = NULL;
     legb.d_dispatch.first = NULL;
     legb.d_dispatch.last  = NULL;
     legb.d_free.first     = &ds_stack[0];
     legb.d_free.last      = &ds_stack[DS_STACK_SIZE - 1];

     /* lega is current right now, after the next step, legb */
     /* will become the new current. Our tx guys will copy   */
     /* legb to lega in the next transaction */

     /* Do the last round! */
     card.op = TX_INIT2;     
     xeris ( X_F_RDC );

     if ( cf->b.p )
     {
          /* again, like before, if something went wrong, */
          /* the Board Pointer would be cleared           */
          cf->b.p = NULL;
          cf->b.s = 0;

          cf->w.s = XST_CPL;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_I_INIT_OK;
          cf->w.t.rpt.cond = RPT_NRM;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Something went wrong, we just pass it on */
     return ( 0 );
}

uint16_t reg ( xdcf * cf )
{
     /* Attempt to register a new cyclic ticket with SIPLEX% */

     xdcf subfile;
     
     /* We need to set up the submission case file */
     subfile.b.p = NULL;   /* This means supervisor agent */
     subfile.b.s = 0;

     subfile.w.d         = X_F_RDC_AGT;
     subfile.w.s         = XST_EVT;
     subfile.w.t.evt.sid = X_F_RDC;
     subfile.w.t.evt.dsk = X_F_RDC_AGT;

     /* Attempt to register it! */
     cf->b.p         = (void *)&subfile;
     cf->b.s         = sizeof ( xdcf );
     cf->w.d         = X_SIPLEX_SUB;
     cf->w.s         = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SUB_LEAS;

     /* Copy permit over */
     cf->w.t.srq.prm.p16[2] = cf->w.t.srq.prm.p16[0];

     /* Set-up particulars */
     cf->w.t.srq.prm.p16[0] = X_F_RDC;    /* Contractor */
     cf->w.t.srq.prm.p16[1] = SHOT_CYC;

     /* That should be it.. */
     xeris ( X_SIPLEX );

     cf->b.p = NULL;
     cf->b.s = 0;

     if ( cf->w.s == XST_ACP )
     {
          /* We're good! */
          cf->w.s = XST_CPL;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_S_OPR_OK;
          cf->w.t.rpt.cond = RPT_NRM;
          cf->w.t.rpt.note = RPT_NON;

          return ( 0 );
     }

     /* Didn't work out... */
     if ( cf->w.s == XST_RJT )
     {
          /* Permit is no good! */
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_NOT_PERMITTED;
     }
     else if ( cf->w.s == XST_UNA )
     {
          /* Not enough free shotcards! */
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_INSUF_RESRC;
     }
     else
     {
          /* This problbably means deny, if not */
          /* it might as well be! */
          cf->w.s = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_NO_AUTH;
     }

     /* finish up the details */
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}
     
               
               
          

     
               

               

     

     
