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
 *  STRAP%
 *  Node-specific Bootstrap Program
 *
 */

#include <xeris.h>
#include <xeris/siplex.h>
#include <xeris/s/clr.h>
#include <xeris/f/rdc.h>

#define X_TIMER 0x0005
#define X_TIMER_INIT 0x0000


uint16_t main ( xdcf * cf )
{
     /* First, let's get interrupts up */
     cf->w.d = X_SIPLEX_SYS;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SYS_INT_INIT;
     xeris ( X_SIPLEX );

     /* Then register our timer and set-all kill */
     cf->w.d = X_SIPLEX_SYS;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SYS_INT_KILL;
     cf->w.t.srq.prm.p08[0] = 0x80; /* Throttling off  */
     cf->w.t.srq.prm.p08[1] = 0x80; /* All-kill set    */
     xeris ( X_SIPLEX );

     /* Intialize our timer */
     cf->w.d = X_TIMER_INIT;
     xeris ( X_TIMER );

     /* Configure designated timer  */
     cf->w.d = X_SIPLEX_SYS;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SYS_INT_TIMR;
     cf->w.t.srq.prm.p16[0] = 16;
     xeris ( X_SIPLEX );
     
     cf->w.d = X_SIPLEX_SYS;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SYS_INT_KILL;
     cf->w.t.srq.prm.p08[0] = 0x80; /* Trottling off */
     cf->w.t.srq.prm.p08[1] = 0x00; /* No-kill */
     xeris ( X_SIPLEX );

     xeris ( X_S_CLR );

     cf->w.s = XST_RPT;
     xeris ( 0 );

     /* Init f_rdc */
     cf->w.d = X_F_RDC_CTL;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_F_RDC_CTL_INIT;
     xeris ( X_F_RDC );

     /* Send out telemetry */
     cf->w.s = XST_TLM;
     cf->w.t.tlm.sec = X_F_RDC;
     xeris ( 0 );


     /* Add an agent */
     cf->w.d = X_F_RDC_CTL;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_F_RDC_CTL_AGENT;
     xeris ( X_F_RDC );

     cf->w.s = XST_TLM;
     cf->w.t.tlm.sec = X_F_RDC;
     xeris ( 0 );

     /* Set up u0 */
     cf->w.s = XST_SRQ;
     xeris ( 0x1235 );

     /* Should be good to go! */
     return ( 0 );
     
}
