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
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  SIPLEX/STARLINE (AVR5)
 *
 *  Dispatch Executor
 *
 */

#include <xeris.h>
#include <core/flatline.h>
#include <core/std/dispatch.h>
#include <core/opsec.h>

#include <siplex/siplex.h>

void bootstrap ( void )
{

     register uint16_t i;

     /* Seed all lanes execept lane zero    */
     /* then do a cold-start on lane zero,  */
     /* which will end us.                  */
     /* exec_crank does not return.         */


     /* Seed all lanes, which allows opsec to */
     /* marshal them. After seeded, the dispatch */
     /* files are released */
     
     for ( i = 0; i < SIPLEX_LANES; i++ )
     {
          exec_seed ( &lanes[i] );
          lanes[i].commission->status = DS_RLS;
     }

     /* lane 1 for cold-start */
     opcon->sched  = lanes[0].commission;
     opcon->active = opcon->sched;

     /* Let there be light. */
     exec_crank ( &lanes[0] );

     return;
}
     
     

void executor ( volatile struct lane * job )
{

     volatile struct shotchain * sabot;
     volatile struct feedline  * hopper;
     
     /* We have just come in from an exec_reset       */
     /* our assigned job file is in the arguments     */
     /* we need to try to pop a sabot from our feed   */
     /* and then fire the shot. If the feeders are    */
     /* all empty, we will set-up a degraded priority */

     /* Signal OPSEC% that we are operating on this   */
     /* dispatch. */
     job->commission->status = DS_EXEC_OP;

     /* Now we can freely modify the dispatch */

     /* First, attempt to pop a sabot */
     sabot = NULL;
     hopper = job->feed;
     
     while ( 1 )
     {

          if ( !hopper )
          {
               /* feeders are all dry!                     */
               /* we need to go into an idle loop and keep */
               /* trying */
               job->commission->status   = DS_EXEC_IDLE;
               job->commission->contract = 1;

               /* OPSEC% will switch to the next dispatch */
               /* and eventually come back to us, when we */
               /* will try again */
               xeris ( 0 );

               job->commission->status = DS_EXEC_OP;

               hopper = job->feed;
               continue;
          }

          sabot = pop_shot ( hopper->feeder );

          if ( !sabot )
          {
               /* this queue is empty */
               hopper = hopper->next;
               continue;
          }

          if ( (sabot->shot->status & 0x0f) == SHOT_HOT )
               /* we have a winner */
               break;

          /* We got a sabot, but its not hot. */
          /* feeders should only ever had hot shots in them */
          /* otherwise, it indicates a shot was canceled,  */
          /* so we help clear it out */

          sabot->shot->status = SHOT_FRE;
          put_shot ( job->recycle, sabot ); 
     }
          

     /* Great, so we have a sabot now, we just need         */
     /* to set up the dispatch file accordingly             */
     /* While the dispatch is set to DS_EXE, OPSEC%         */
     /* will not read of modify any of the file parameters  */
     /* on a preemption, except for the zone file, which we */
     /* do not access.                                      */

     /* First attach the dispatch to the new master case    */
     /* file */
     /* We are setting the casefile in place now, so it's   */
     /* no-longer volatile for our purposes (out of our     */
     /* "jusrisdiction" from here-on-out.                   */
     job->commission->cf = (xdcf *)&sabot->shot->master;

     /* First, we need initialize the dispatch file with    */
     /* the Operational Certificats for both the client and */
     /* contractor listed on the card. From this we can     */
     /* inform the default priority setting, and also       */
     /* ensure the dispatch is prepared for execution       */

     /* Since  we have set the dispatch to EXEC_OP, OPSEC%  */
     /* will do a "dry-run" if we do a syscall, so we can   */
     /* get the right soc in super->cisec */
     xeris ( sabot->shot->client );

     /* Set up dispatch parameters */
     job->commission->priority = job->commission->super->cisec.bpri;
     job->commission->deadline = sicon.def_deadline;
     /* Note - we don't reset the dispatch claim, as this is not   */
     /* really our jurisdiction. XS will handle that at end of day */
     /* This protects from really fast cyclic tickets from causing */
     /* the lane to hog the schedule constantly. */
     job->commission->contract = sicon.def_contract;
     job->commission->sideline = 0;

     /* Now one last thing before we go.. If the client    */
     /* and contractor are the same, which could be pretty */
     /* common, if we do nothing, it would cause OPSEC% to */
     /* attempt to enter a transaction. To avoid this, we  */
     /* can set cisec.sid to zero, which signals OPSEC%    */
     /* This works great since pretty much only we can     */
     /* actually pull that off.. */
     if ( sabot->shot->client == sabot->shot->contractor )
          job->commission->super->cisec.sid = 0;

     /* Release dispatch */
     job->commission->status = DS_EXE;
     /* That's it, we're ready to fire */

     sabot->shot->status = SHOT_FIR;
     xeris ( sabot->shot->contractor );

     /* Dispatch is complete */
     job->commission->status = DS_EXEC_OP;
     job->commission->cf     = NULL;

     /* First check if shot is cyclic. If so, we put */
     /* it right back in the queue from whence it came */
     if ( (sabot->shot->intent & 0x0f) == SHOT_CYC )
     {
          sabot->shot->status = SHOT_HOT;
          put_shot ( hopper->feeder, sabot );
          exec_crank ( job );
     }

     /* Check if shot is private */
     if ( sabot->shot->intent & SHOT_PRV )
     {
          /* Shot is private, we unload and forget      */
          /* sabot gets "ejected" */
          sabot->shot->status = SHOT_UNL;
          sabot->shot = NULL;

          /* recycle sabot */
          put_shot ( job->eject, sabot );

          /* We're done here */
          /* Call for reset */

          exec_crank ( job );
     }

     /* We have a public shot */
     /*santize the Case File */
     sabot->shot->master.b.p          = NULL;
     sabot->shot->master.b.s          = 0;
     sabot->shot->master.w.d          = 0;
     sabot->shot->master.w.s          = 0;
     sabot->shot->master.w.t.p164.seq = 0;
     sabot->shot->master.w.t.p164.pld = 0;
     
     /* mark the shot as fre and recycle */
     sabot->shot->status = SHOT_FRE;
     put_shot ( job->recycle, sabot );
     exec_crank ( job );

     return;
}
