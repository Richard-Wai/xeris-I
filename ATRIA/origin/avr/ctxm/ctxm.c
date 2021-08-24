/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  All Rights Reserved.
 */

/*
 *  XERIS CORE
 *
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  ATRIA/ORIGIN (AVR)
 *
 *  CTXM$
 *  Context Manager 
 *
 *  Primary Operator
 *  ctxm.c
 */

#include <xeris.h>
#include <core/atria.h>

#include <atria/ctxm.h>

uint16_t atria_ctxm ( xdcf * cf )
{
     /* Route to secondary operator */

     if ( cf->w.s != XST_SRQ )
     {
          /* We only accept service requests! */
          cf->w.s = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     switch ( cf->w.t.srq.rqc )
     {
          case ATRIA_CTXM_GEN:
               atria_ctxm_initgen ( cf );
               return ( 0 );
               break;

          default:
                break;
     }

     /* Bad request code */
     cf->w.s = XST_UNA;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}
