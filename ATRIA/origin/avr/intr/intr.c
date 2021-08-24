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
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  ATRIA/ORIGIN (AVR)
 *
 *  INTR$
 *  Interrupt Service Registrar 
 *
 *  Service Desk
 *  intr.c
 */

#include <xeris.h>
#include <core/atria.h>

#include <atria/intr.h>

uint16_t atria_intr ( xdcf * cf )
{
     /* We take service tickets only */
     if ( cf->w.s != XST_SRQ )
     {
          cf->w.s          = XST_RPT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }
     
     /* route service */
     /* At this time, we really only have one service */
     
     if ( cf->w.t.srq.rqc == SVC_ATRIA_INTR_REFISRT )
     {
          /* Pull ISRT reference pointer */
          /* Pointer is returned as a XERIS Standard Message */
          /* ticket with a 64-bit argument, which is system */
          /* specific, but universally handled by by OPSEC%INT$ */
          
          /* In our case (avr), it is just a memx pointer cast */
          /* to uint64_t */
          
          /* Ref zero means "local normal pointer" */
          
          cf->w.s = XST_MSG;
          cf->w.t.msg.ref = 0;
          
          atria_intr_sisr ( &cf->w.t.msg.arg.a64 );
          
          return ( 0 );          
     }


     /* no service */
     cf->w.s          = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}
