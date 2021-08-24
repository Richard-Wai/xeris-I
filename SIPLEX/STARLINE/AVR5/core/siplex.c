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
 *  Dispatch Desk
 *
 */

#include <xeris.h>

#include <siplex/siplex.h>


uint16_t siplex ( xdcf * cf )
{

     if ( cf->w.s != XST_SRQ )
     {
          /* Invalid submission */
          cf->w.s          = XST_RPT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Route to appropriate operator */
     if ( cf->w.d == X_SIPLEX_SUB )
          /* sub$ - Shot Submission Desk */
          return ( subdesk ( cf ) );
     else if ( cf->w.d == X_SIPLEX_SYS)
          /* sys$ - System desk */
          return ( sysdesk ( cf ) );

     /* No operator */
     cf->w.s          = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );

}

     

