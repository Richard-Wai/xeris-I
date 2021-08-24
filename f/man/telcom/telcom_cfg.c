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
 *  XERIS Support Group
 *
 *  man%
 *  Human Relations Secretariat
 *
 *  ttu$
 *  Telemetry Translation Unit
 *
 *  Configuration Service Assistant
 *  ttu_cfg.c
 *
 */

#include <xeris.h>

#include <man/man.h>
#include <man/telcom.h>

struct tc_cfg_s tc_cfg;

void telcom_cfg ( xdcf * cf )
{
     /* First we need to check for sufficient downstream */
     /* security from the origin */

     if ( cf->s.sec < tc_cfg.cfg_auth )
     {
          /* No-go */
          cf->w.s = XST_DNY;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;

          return;
     }

     /* Determine service modifier */
     if ( cf->w.t.srq.rqc == TELCOM_CFG_CON )
     {
          /* Console Config */
          tc_cfg.console_sid = cf->w.t.srq.prm.p16[0];
          tc_cfg.console_dsk = cf->w.t.srq.prm.p16[1];
     }
     else if ( cf->w.t.srq.rqc == TELCOM_CFG_SEC )
          /* Config downstream security level minimum */
          tc_cfg.cfg_auth = cf->w.t.srq.prm.p16[0];


     cf->w.s = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_ACCEPTED;
     cf->w.t.rpt.cond = RPT_OKY;
     cf->w.t.rpt.note = RPT_NON;

     return;
}
     
     
