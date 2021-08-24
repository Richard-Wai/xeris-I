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
 *  FLATLINE%SYSBOOT$
 *  System Bootstrap Operator
 *
 */

#include <xeris.h>
#include <sys/m.h>
#include <std/tlm/flatline.h>
#include <core/std/dispatch.h>
#include <core/flatline.h>
#include <core/opsec.h>
#include <core/atria.h>

#include <flatline/flatline.h>


void flatline_sysboot ( xdcf *cf )
{

     /* The primary role of SYSBOOT% is to set-up a bare-bones */
     /* dispatch file on the stack so that we can support a    */
     /* "fully functional" but single-dispatch system until    */
     /* SIPLEX% is brought up. SIPLEX% will then destory this  */
     /* dispatch, and reconfigure the system properly.         */

     volatile struct opsec_opcon * opcon;
     
     volatile        dispatch          pd;
     volatile struct dispatch_zone     pzone;
     volatile struct dispatch_super    psuper;
     volatile struct dispatch_transact ptransact;

     pd.status    = DS_EXE;
     pd.priority  = 0xffff;
     pd.deadline  = 0xffff;
     pd.contract  = 0xff;
     pd.claim     = 0x00;
     pd.sideline  = 0x0000;

     pzone.base   = (void *)(c_RAMEND);
     pzone.shock  = (void *)(c_RAMSTART);
     pzone.thresh = pzone.shock;
     pzone.slim   = pzone.shock;
     pzone.smark  = pzone.base;
     pzone.stat   = DZ_ZONE_RDY;

     psuper.cisec.sid     = XSEC_FLATLINE;
     psuper.cisec.dd      = &flatline;
     psuper.cisec.bpri    = 0xffff;
     psuper.cisec.dsec    = 0xffff;
     psuper.cisec.usec    = 0xffff;
     psuper.cisec.account = NULL;
     psuper.termret       = (void *)(0x0000);
     psuper.termmrk       = (void *)(c_RAMEND);
     psuper.exec          = NULL;

     ptransact.exec    = NULL;
     ptransact.mark    = NULL;
     ptransact.ctarget = 0;
     ptransact.amark   = 0;

     pd.cf        = cf;
     pd.zone      = &pzone;
     pd.super     = &psuper;
     pd.nx_binder = NULL;
     pd.nx_sched  = NULL;
     pd.transact  = &ptransact;
     
     /* Set opcon pointer */
     opcon = (struct opsec_opcon *)(cf->b.p);
     
     /* Configure opcon with dispatch */
     opcon->active  = &pd;
     /* Make sure the scheduler does not run */
     opcon->mission = opcon->active;
     opcon->sched   = NULL;
     opcon->xs_in   = 0;
     opcon->xs_tick = 0;
     opcon->p_tick  = 0;
     opcon->p_soft  = 0;

     opcon->telcom  = 0;


     /* ok, we're done with our job, call OMNI */
     /* omni is just using our case file for the temporary disaptch */
     /* Give omni our stack buffer too */
     cf->b.p = (void *)opcon;
     flatline_omni ( cf );

     /* omni should pass control to SIPLEX%, which will destroy */
     /* the stack, returning here should be impossible, but  */
     /* just in case, try to notify the parent, something is  */
     /* really wrong */

     cf->w.s = XST_RPT;
     cf->w.t.rpt.spec      = XSR_OP;
     cf->w.t.rpt.code      = XSR_OP_F_INIT_FATAL;
     cf->w.t.rpt.adi.adi32 = 0;

     xeris ( 0 );
     xeris ( 0 );
     xeris ( 0 );

     while ( 1 );
}


     
