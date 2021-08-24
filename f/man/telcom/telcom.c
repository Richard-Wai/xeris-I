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
 *  telcom$
 *  Telemetry Commissioner
 *
 *  Service Desk
 *
 */

#include <xeris.h>
#include <xeris/s/rsv.h>
#include <xeris/s/isu.h>

#include <man/man.h>
#include <man/internal.h>
#include <man/telcom.h>
#include <man/std/man.h>


/* Assistant Modules */
extern void telcom_cfg    ( xdcf * cf );

uint16_t man_telcom ( xdcf * cf )
{
     unsigned char buf[BUFSTD];
     xdcf scase;
     
     /* Casefile is not pre-screened.             */
     /* First step is to check for an accceptable */
     /* stub */

     if ( cf->w.s == XST_SRQ )
     {
          /* Config Ticket */
          telcom_cfg ( cf );
          return ( 0 );
     }
          
     if ( cf->w.s != XST_TLM )
     {
          /* No good. Abort */
          return ( 0 );
     }

     /* Save the case board */

     scase.b.p = cf->b.p;
     scase.b.s = cf->b.s;
     cf->b.p = &scase;
     cf->b.s = sizeof ( xdcf );
     xeris ( X_S_RSV );

     /* Set-up case file for processing by STU$ */
     cf->b.p = (void *)buf;
     cf->b.s = BUFSTD;

     man_stu ( cf );

     /* Send result out */
     cf->w.d = tc_cfg.console_dsk;
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = 0;       /* universal TX String */
     xeris ( tc_cfg.console_sid );

     /* Restore board */
     cf->b.p = &scase;
     cf->b.s = sizeof ( xdcf );
     xeris ( X_S_ISU );

     return ( 0 );
}
          


     


