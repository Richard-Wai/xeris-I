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
 *  System Interface Desk
 *  SYS$
 *
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <core/opsec.h>

#include <siplex/siplex.h>
#include <siplex/int.h>

uint16_t sysdesk ( xdcf * cf )
{
     if ( cf->s.sec <= sicon.sys_sec )
     {
          /* Insufficient security */
          cf->w.s = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;

          return ( 0 );
     }

     switch ( cf->w.t.srq.rqc )
     {
          case X_SIPLEX_SYS_INIT:
               return siplex_init ( cf );
               break;

          case X_SIPLEX_SYS_INT_INIT:
               /* Initialize interrupt subsystem */
               return siplex_int_init ( cf );
               break;

          case X_SIPLEX_SYS_INT_KILL:
               /* Interrupt Kill flag control */
               return siplex_int_kill ( cf );
               break;

          case X_SIPLEX_SYS_INT_TIMR:
               /* Preemption "designated timer" config */
               return siplex_int_timr ( cf );
               break;
          
          case X_SIPLEX_SYS_INT_SUPR:
               /* Interrupt supervisor */
               return siplex_int_supr ( cf );
               break;

          default:
               break;
     }

     cf->w.s          = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     
     return ( 0 );

}


