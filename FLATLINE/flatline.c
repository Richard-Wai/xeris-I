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
 *  FLATLINE%
 *  First Load Assesment Telemetry, Link Initialization for
 *  Nominal Entry
 *
 *  FLATLINE/flatline.c
 *  Core Module
 */

#include <xeris.h>
#include <core/flatline.h>

#include <flatline/flatline.h>

/* FLATLINE% Dispatch Desk */

uint16_t flatline ( xdcf * cf )
{
     /* FLATLINE only has static operators.              */
     /* All we really need to do is put the approriate   */
     /* operator on the case.                            */

     switch ( cf->w.d )
     {
          case OP_FLATLINE_ULIF:
               return ( flatline_ulif ( cf ) );
               break;

          case OP_FLATLINE_SYSBOOT:
               /* sysboot doesn't return! */
               flatline_sysboot ( cf );
               break;

          default:
               break;
     }

     /* Track is not recognised, or other error          */
     /* FLATLINE% does not accept dispatches itself      */
     /* SYSBOOT$ is not allowed to receive dispatches    */
     /* Therefore if we get here, we REJECT the dispatch */

     cf->w.s               = XST_RJT;  /* Rejected */
     cf->w.t.rpt.spec      = XSR_OP;
     cf->w.t.rpt.code      = XSR_OP_F_NO_OPERATOR;
     cf->w.t.rpt.cond      = RPT_FTL;
     cf->w.t.rpt.note      = RPT_NON;
     cf->w.t.rpt.adi.adi32 = 0;

     
     return ( 0 );
}
