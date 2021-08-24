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
 *  OS$
 *  Operational Scheduler
 *
 *  End of Day Proceesing
 *  opsec/os/os_eod.c
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <core/opsec.h>

#include <opsec/opsec.h>


void opsec_os_eod ( void )
{
     volatile dispatch * finger = opcon.mission;

     while ( finger )
     {
          /* Clear out the old schedule */
          finger->nx_sched = NULL;
          
          /* Check for out-cycle conditions */
          if ( finger->status >= DS_EXEC_EMRG )
          {
               /* Either this dispatch is out of     */
               /* service, or (more likely) it's     */
               /* Executor is operating, in which    */
               /* case the scheduler will handle it  */
               /* Otherwise, if it is out of service */
               /* it just get skipped entirely       */

               /* One thing we should do, however,   */
               /* is ensure the priority is set to   */
               /* minimum */
               finger->priority = 0;

               /* For us, we just hop over it  */
               finger = finger->nx_binder;
               continue;
          }

          /* Check for sidelined dispatches */
          /* (didn't get a go last cycle)   */
          if ( finger->status == DS_SCH )
               /* increment the sideline counter */
               finger->sideline++;

          /* start as a slack dispatch */
          finger->status = DS_SLK;
               
          
          /* Contract claim reset */
          finger->claim = 0;

          if ( finger->deadline > 0 )
          {
               /* We have a mainline dispatch, find out */
               /* what kind it is */
               
               /* First, update deadline */
               finger->deadline--;

               if ( finger->deadline < opcon.dllimt )
               {
                    /* Deadline getting too close! */
                    /* boost condition */
                    if ( finger->contract < (0xffff - opcon.dlbump) )
                         /* you never know.... */
                         finger->contract += opcon.dlbump;                    

                    /* extra priority boost */
                    finger->status = DS_DEX;
               }
               else if ( finger->deadline < opcon.dlmark )
               {
                    /* Dispatch is approaching deadline */
                    /* elevate to deadline approach phase */
                    finger->status = DS_DAP;
               }
               else if ( finger->sideline >= opcon.prothr )
               {             
                    /* too long on the sidelines */
                    finger->sideline = 0;
                    finger->status = DS_PRO;
               }
          }
          else
          {
               /* When a dispatch missed it's deadline  */
               /* it effectivly gets demoted to being   */
               /* regular dispatch, but with a minimum  */
               /* claim */

               /* If it happens to be a transaction head */
               /* we will wait until it closes-out */

               /* Contract gets reset to minimum */
               finger->contract = 1;
          }

          finger = finger->nx_binder;
     }

     return;
}
               

          
     
