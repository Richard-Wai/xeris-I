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
 *  OPSEC%
 *  Operational Secretariat
 *
 *  SYS$
 *  Local System Procedures
 *
 *  opsec/sys/sys.c
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <core/flatline.h>
#include <core/opsec.h>
#include <opsec/opsec.h>
#include <opsec/sys.h>
#include <opsec/int.h>


/*                                   */
/* System "flatline"/reset entry.    */
/* set-up priordial dispatch on      */
/* stack, and boot to FLATLINE%      */
/* This proceedure should never exit */
/* since SIPLEX% will wipe the state */
/*                                   */

void sys_boot ( void )
{
     /* Very basic initialization of opcon and opint    */
     /* we really can't get to fancy yet, and we can't  */
     /* go through ourselvs to get there, we gotta just */
     /* link opint to opcon, and pass the location of   */
     /* opcon up to FLATLINE%. We don't know what kind  */
     /* of things FLATLINE% needs to do to get our      */
     /* facilities up */


     xdcf planck;

     /* Make sure interrupts are reset */


     opint.sbase   = NULL;
     opint.slim    = NULL;
     opint.smark   = NULL;

     opint.sigpre  = 0x00;
     opint.throtl  = 0x7f;    /* no nesting */
     opint.kilmsk  = 0x80;    /* all-kill */

     opint.msg_q   = NULL;
     opint.msg_max = NULL;

     opint.sisr    = 0;

     /* Link to opcon */
     opcon.intr    = &opint;
     

     while ( 1 )
     {
          
          planck.b.p = (void *)(&opcon);
          planck.b.s = sizeof (struct opsec_opcon);
          planck.w.d = OP_FLATLINE_SYSBOOT;

          sys_flatline ( &planck );
     }

     return;
}
     


     

     

     
     
