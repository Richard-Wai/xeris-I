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
 *  opsec/os/os.c
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <core/opsec.h>
#include <opsec/opsec.h>

extern void opsec_os_eod ( void );

void opsec_os ( void )
{
     /* "strands" */
     /* These are ordered by general priority, */
     /* highest to lowest.                     */

     /* There are 4 strands, strand #0 is highest priority */
     /* Strand #3 is the slack strand */
     /* Why 5? Because: */
     /* 0: Deadline Expadite        */
     /* 1: Deadline Approach        */
     /* 2: Anti-sideline Promotions */
     /* 3: Slack                    */

     /* Operating Executors get special treatment. */
     /* Normally they set their own priority to    */
     /* Minimum, and then get put into stand 1     */
     /* Emergency Executor Ops go into strand 0    */

     /* Here are the strands, both ends */
     volatile dispatch * strand_head[4] =
          { NULL, NULL, NULL, NULL };
     volatile dispatch * strand_tail[4] =
          { NULL, NULL, NULL, NULL };

     volatile dispatch *splice, *finger;
     volatile dispatch **select, **mark;

     uint8_t i, j;

     
     /* First order of business is to update all of our   */
     /* accounting info. And update the statuses, etc. To */
     /* stream-line things, we'll put  this in another    */
     /* function to allow for modularity  */
     opsec_os_eod ( );

     /* Next we undergo a two-stage process to create     */
     /* the new schedule:                                 */
     /* 1. Sort and arrange all mission dispatches onto   */
     /*    their appropriate schedule strand              */
     /* 3. Splice strands according to heiarchy           */

     /*** STAGE 1 ***/
     finger = opcon.mission;
     select = NULL;
     mark   = NULL;
     
     while ( finger )
     {
          /* os_eod () ensures all dispatches are       */
          /* either > DS_EXEC_IDLE, or are valid        */
          /* scheduling codes (DEX/DAP/PRO/SLK)         */
          /* or valid Executor codes (EMERG/OP/IDLE)    */
          /* so we only need to screen for non-schedule */
          /* dispatches */
          
          if ( finger->status > DS_EXEC_IDLE )
          {
               /* Not scheduled */
               finger = finger->nx_binder;
               continue;
          }

          /* Executor status codes are paired with regular    */
          /* scheduling codes such that the destination       */
          /* strand is encoded by the low nibble of the code  */
          /* (with an offset of one above DS_RLS). This means */
          /* if we mask the high nibble and subtract DS_RLS,  */
          /* we will get the strand index number + 1.         */
          /* Very efficient. */

          i = ( (finger->status & 0x0f) - (DS_RLS + 1) );

          select = &strand_head[i];
          mark   = &strand_tail[i];


          /* So now we have select pointing to the strand  */
          /* to which finger should be attached. We need   */
          /* to find where on the strand to actually place */
          /* the selected dispatch */

          if ( *select == NULL )
          {
               
               /* Nothing on this strand yet! */
               *select = finger;
               *mark = finger;
               finger = finger->nx_binder;
               continue;
          }

          /* First check the priority of the head of the strand. */
          /* if it is lower than finger, we put finger at the    */
          /* top, if it is higher or equal, we walk the strand   */
          /* until we find one with higher deadline than us, or  */
          /* a lower priority  */
          if ( (*select)->priority < finger->priority )
          {
               finger->nx_sched = *select;
               *select = finger;
          }
          else
          {
               /* we know we need to walk until we find:      */
               /* A) lower priority, or B) empty slot */
               /* B) lower priority, C) empty slot    */
               splice = *select;

               /* make sure the stand is more than 1 long.. */
               while ( splice->nx_sched )
               {
                    if (splice->nx_sched->priority <= finger->priority)
                         break;

                    /* walk it down */
                    splice = splice->nx_sched;
                         
               }

               /* splice in before the splice dispatch ( if any ) */
               if ( splice->nx_sched )
                    finger->nx_sched = splice->nx_sched;
               
               splice->nx_sched = finger;
               
               if ( splice == *mark )
                    /* make sure we update the strand end */
                    /* if splice is an end-piece */
                    *mark = finger;
               
          }

          /* mark dispatch as scheduled and move on */
          finger->status = DS_SCH;
          finger = finger->nx_binder;
     }


     /*** STAGE 3 ***/
     /* Link the stands, end-to-end */
     /* we can just go by tail-to-head conntection */
     /* and try them down the line */

     opcon.sched = strand_head[0];

     for ( j = 0, i = 1; i < 4; i++ )
     {
          /* j is the "tail-side" connection */
          /* i is the "head-side" connection */

          if ( !strand_tail[j] )
          {
               /* empty strand at front, skip */
               opcon.sched = strand_head[++j];
               --i;
               continue;
          }

          
          strand_tail[j]->nx_sched = strand_head[i];

          if ( strand_tail[j]->nx_sched )
               /* good join */
               j = i;
     }

     /* if all strands are null, we have no       */
     /* scheduable dispatches at all. This could  */
     /* only really happen in a really bad system */
     /* intervention event.. we just drop it back */
     /* null to XS$, and it'll get back to CDD$   */
     /* so we don't need to actually do anything  */
     /* it is known that opcon.sched can be null  */


     /* long day ahead! let's get to work */

     return;
}
               
          

          
               
          
     
     
                         
                              
                         
                    
          
               
          
          

                    
                    
                    
               
               
     

     
     
     

     
