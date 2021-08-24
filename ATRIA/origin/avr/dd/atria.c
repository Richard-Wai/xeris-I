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
 *  Dispatch Desk
 *
 *  <atria/secreg.h>
 */

#include <xeris.h>
#include <sys/types.h>

#include <core/atria.h>

#include <atria/secreg.h>
#include <atria/ctxm.h>
#include <atria/intr.h>

/* uint16_t atria_ldl[ATRIA_LDL_LEN]; */

uint16_t main ( xdcf *cf )
{
     /* Route the Case File to the appropriate */
     /* Operator */

     switch ( cf->w.d )
     {
          case OP_ATRIA_SECREG:
               return ( atria_secreg ( cf ) );
               break;

          case OP_ATRIA_CTXM:
               return ( atria_ctxm ( cf ) );
               break;

          case OP_ATRIA_INTR:
               return ( atria_intr ( cf ) );
               break;

          default:
               break;

     }

     /* No OP, set-up report ticket */
     cf->w.s = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}               
          
