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
 *  SECREG$
 *  Secretariat Registrar 
 *
 *  Operator of Record
 *  secreg.c
 */

#include <xeris.h>
#include <core/atria.h>

#include <atria/secreg.h>

uint16_t atria_secreg ( xdcf * cf )
{
     /* We only take Service Request Tickets*/
     if ( cf->w.s != XST_SRQ )
     {
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_CNC;
          cf->w.t.rpt.note = RPT_NON;
          return ( 0 );
     }

     /* Now try to pass it to the correct assistant */
     /* Right now we only have one service.. */

     if ( cf->w.t.srq.rqc == ATRIA_SECREG_OPCERT )
     {
          atria_secreg_svc_opcert ( cf );
          return ( 0 );
     }


     /* Service Code not Recognised */
     cf->w.s          = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_SVC_UNAVAIL;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}
          
     
