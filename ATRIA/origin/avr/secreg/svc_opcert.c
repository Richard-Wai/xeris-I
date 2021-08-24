/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All Rights Reserved.
 */

/*
 *  XERIS CORE
 *
 *  ATRIA%
 *  Authorized Track Registrar and Integration Agent
 *
 *  DESK$
 *  ATRIA Desk Card Retreival Service
 *
 */

#include <xeris.h>
#include <core/atria.h>

#include <atria/secreg.h>


/* SECREG$[SVC]OPCERT Service Handler */

void atria_secreg_svc_opcert ( xdcf * cf )
{
     /* Quite frankly a super-simple operation           */
     /* First we scan the lockdown list for matches      */
     /* If we don't find any, we try to pull the file    */
     /* If we find the file, we fill out the certificate */

     /* First, we will check that the Work Board is properly */
     /* configured */
     if
     (
          (cf->b.s != sizeof (struct atria_soc)) ||
          (!cf->b.p)
     )
     {
          /* Bad board config, abort with a hint */
          cf->w.s               = XST_RJT;
          cf->w.t.rpt.spec      = XSR_OP;
          cf->w.t.rpt.code      = XSR_OP_F_CB_CONFIG;
          cf->w.t.rpt.cond      = RPT_RTY;
          cf->w.t.rpt.note      = RPT_REF;
          cf->w.t.rpt.adi.adi32 = sizeof (struct atria_soc);
          return;
     }

     /* Next we attempt to pull the Registration File */
     if
     (
          !atria_secreg_pullsoc ( cf->w.t.srq.prm.p16[0],
                                  (struct atria_soc *)cf->b.p )
     )
     {
          /* No such secretariat.. */
          cf->w.s               = XST_UNA;
          cf->w.t.rpt.spec      = XSR_OP;
          cf->w.t.rpt.code      = XSR_OP_F_NO_SECRETARIAT;
          cf->w.t.rpt.cond      = RPT_FTL;
          cf->w.t.rpt.note      = RPT_NON;
          return;
     }

     /* Looks good! */
     /* Set up report ticket */
     cf->w.s               = XST_CPL;
     cf->w.t.rpt.spec      = XSR_OP;
     cf->w.t.rpt.code      = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond      = RPT_NRM;
     cf->w.t.rpt.note      = RPT_NON;
     return;
     
}
     

