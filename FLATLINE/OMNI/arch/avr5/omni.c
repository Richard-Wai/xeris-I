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
 *  FLATLINE%OMNI$
 *  Operational Minimum Node Initialization
 *
 *  FLATLINE/OMNI/arch/avr5/omni.c
 *  OMNI$ For AVR5
 */

#include <xeris.h>
#include <xeris/siplex.h>
#include <xeris/f/man.h>
#include <sys/m.h>
#include <std/tlm/flatline.h>
#include <core/flatline.h>
#include <core/opsec.h>
#include <core/std/dispatch.h>

#include <flatline/flatline.h>

#define XSEC_MAN       0x1234
#define OP_MAN_TELCOM  0x8000

#define SVC_MAN_TELCOM_CON  0x0000
#define SVC_MAN_TELCOM_SEC  0x0001

static void cfg_telcom ( xdcf *, volatile struct opsec_opcon * );

uint16_t flatline_omni ( xdcf *cf )
{
     volatile struct opsec_opcon * opcon;
     
     uint16_t ulif_lines;               /* ULIF$ Init results */

     /* Load in opcon pointer */
     opcon = (volatile struct opsec_opcon *)cf->b.p;
     cf->b.p = NULL;

     /* First thing that must be done is to bring up ULIF$ */          
     /* Set-up dispatch order init ULIF$ */
     /* Stub */
     cf->w.d         = OP_FLATLINE_ULIF;   /* FLATLINE%ULIF$     */
     cf->w.s         = XST_SRQ;            /* Service Request    */
     cf->w.t.srq.rqc = ULIF_INI;           /* Init interface RQC */

     flatline_ulif ( cf );                 /* Submit dispatch    */

     ulif_lines = cf->w.t.rpt.adi.adi16[0];   /* Get lines count */


     /* Attach telcom (the first time) */
     opcon->telcom = 0xffff;
     cfg_telcom ( cf, opcon );
     
     /* Telcom should now be up, so we can start by */
     /* Exporting our telemetry up to this point    */

     /* Inception notice (x2) */
     cf->w.s          = XST_RPT;
     cf->w.t.rpt.spec = XSR_TLM_FLATLINE;
     cf->w.t.rpt.code = TLM_FLATLINE;
     xeris ( 0 );
     xeris ( 0 );

     /* "%ULIF " */
     cf->w.t.rpt.code = TLM_ULIF;
     /* its easier to save uint8[2] as uint16 */
     cf->w.t.rpt.adi.adi16[0] = ulif_lines;
     xeris ( 0 );

     /* %OMNI $NOMINAL */
     cf->w.t.rpt.code = TLM_OMNI_NOM;
     xeris ( 0 );

     /* Next, we should state the total system memory */
     cf->w.t.rpt.code = TLM_OMNI_RMEM_0;
     cf->w.t.rpt.adi.adi32 = (c_RAMEND - c_RAMSTART) + 1;
     xeris ( 0 );
     

     /* Next, we need to pass opcon to ATRIA and     */
     /* ask for ATRIA%CTXM$ to init working memory   */
     /* ATRIA% will also configure the dispatch file */
     /* for SIPLEX% to know what it can work with    */

     /* Set-up work-order to atria */
     cf->b.p                = (void *)opcon;
     cf->w.d                = OP_ATRIA_CTXM;
     cf->w.s                = XST_SRQ;
     cf->w.t.srq.rqc        = ATRIA_CTXM_GEN;
     cf->w.t.srq.prm.p16[0] = ATRIA_CTXM_GEN_NOMINAL;
     xeris ( XSEC_ATRIA );

     /* ATRIA very likely initialized the memory of   */
     /* telcom, which means we need to reconfigure it */
     cfg_telcom ( cf, opcon );

     /* Now state the computed executive memory */
     /* ATRIA% already allocated all executive memory */
     /* to the active zone file */
     cf->w.s               = XST_RPT;
     cf->w.t.rpt.spec      = XSR_TLM_FLATLINE;
     cf->w.t.rpt.code      = TLM_OMNI_XMEM_0;
     cf->w.t.rpt.adi.adi32 = (uint32_t)
          ( (opcon->active->zone->base -
             opcon->active->zone->shock) + 1);
     xeris ( 0 );

     /* Finally, we print the XERIS banner before */
     /* booting to SIPLEX */

     /* Release */
     cf->w.t.rpt.code = TLM_XERIS_REL;
     cf->w.t.rpt.adi.adi32 = 1;
     xeris ( 0 );

     /* Major version */
     cf->w.t.rpt.code = TLM_XERIS_V_MAJ;
     cf->w.t.rpt.adi.adi32 = 1;
     xeris ( 0 );

     /* Minor version */
     cf->w.t.rpt.code = TLM_XERIS_V_MIN;
     cf->w.t.rpt.adi.adi32 = 0;
     xeris ( 0 );

     /* Copyright */
     cf->w.t.rpt.code = TLM_XERIS_COPY;
     cf->w.t.rpt.adi.adi32 = 2017;
     xeris ( 0 );

     /* Legal */
     cf->w.t.rpt.code = TLM_XERIS_LEG1;
     xeris ( 0 );

     /* call SIPLEX%INIT$ to get things started */
     cf->b.p = (void *)opcon;
     cf->w.d = X_SIPLEX_SYS;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = X_SIPLEX_SYS_INIT;
     xeris ( X_SIPLEX );

     return ( 0 );
}


static void cfg_telcom ( xdcf * cf, volatile struct opsec_opcon * opcon )
{
     /* Intervention */
     cf->w.d = 0x0000;
     cf->w.s = XST_SRQ;
     xeris ( 0x1236 );
     opcon->telcom = 0x1236;
     return;
     
     
     /* Configure telcom. In our case, we will use MAN%TC$ */
     /* so first, we need to configure MAN%TC$ to to use ULIF%   */
     /* Set "console" to ULIF$ */
     cf->w.d                = OP_MAN_TELCOM;
     cf->w.s                = XST_SRQ;
     cf->w.t.srq.rqc        = SVC_MAN_TELCOM_CON;
     cf->w.t.srq.prm.p16[0] = XSEC_FLATLINE;      /* Console SID  */
     cf->w.t.srq.prm.p16[1] = OP_FLATLINE_ULIF;   /* Console Desk */
     xeris ( XSEC_MAN );

     /* Now set the security to the higest levels to */
     /* ensure not just anyone can config MAN%TC$ */
     cf->w.d                = OP_MAN_TELCOM;
     cf->w.s                = XST_SRQ;
     cf->w.t.srq.rqc        = SVC_MAN_TELCOM_SEC;
     cf->w.t.srq.prm.p16[0] = 0xffff;
     xeris ( XSEC_MAN );
     
     /* Configure opsec to use the specified telcom */
     /* Note that all official telcoms must be at desk 0x8000 */
     /* MAN%TC$, of course, is at desk 0x8000 */
     opcon->telcom = XSEC_MAN;

     return;
}
