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
 *  XS$
 *  Executive Scheduler
 *
 *  opsec/xs/xs.c
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <opsec/opsec.h>
#include <opsec/xdm.h>
#include <opsec/os.h>
#include <opsec/xs.h>
#include <opsec/tx.h>

/* Atomic ops via ASM */
extern uint8_t lock_xs  ( volatile uint8_t * xs_in );
extern uint8_t free_xs  ( volatile uint8_t * xs_in );
extern void clear_atomic16  ( volatile uint16_t * target );

/* Check for tx interruption, and execute roll-back */
/* if need-be */
static void checktx ( void );

void opsec_xs ( void )
{
     /* First try to make sure we're the only one */
     /* trying to do this.. */

     if ( !lock_xs ( &opcon.xs_in ) )
          /* We need to step aside */
          return;

     /* We're in, new slot, claim was made */
     opcon.active->claim++;
     opcon.xs_tick++;
     
     /* first order of buisness is to see if we're at the end */
     /* of day. opcon.xs_tick tells us how many slots have    */
     /* passed today, and if that number is greater than the  */
     /* schedule slots per day (opcon.sslots), we need to do  */
     /* our "end of day", and generate an updated schedule    */
     /* this is handled by the Operational Scheduler (OS$)    */
     if ( opcon.xs_tick > opcon.sslots )
     {
          /* get the new schedule */
          opsec_os ( );

           if ( opcon.sched == NULL )
           {
                /* Schedule is empty, which could happen for     */
                /* a number or reasons.. many of which are       */
                /* bad, and we need to get control back to cdd$  */
                /* to keep working on it, or even back to SIPLEX */
                /* we will ask os$ to draw up a new schedule     */
                /* we need to abort the schedule change */
                opcon.xs_tick++;
                free_xs ( &opcon.xs_in );
                return;
           }     

          /* ready the new schedule */
          opcon.sched->claim = 0;

          /* new day */
          opcon.xs_tick = 0;
          clear_atomic16 ( &opcon.p_tick );

          /* We need to boot the new day in at the top      */
          /* (we need to marshal in to head of the schedule */
          opsec_xdm ( opcon.sched );

          if ( opcon.active->transact->mark )
               /* This dispatch is in a transaction    */
               /* check the integrity before alllowing */
               /* it to continue. We might not return  */
               /* checktx will free_xs if a rollback   */
               /* is needed */
               checktx ( );
          
          free_xs ( &opcon.xs_in );
          return;
          
     }



     /* check if the dispatch exhausted it's contract claim yet*/
     /* if it hasn't, we just continue-on through */

     if ( opcon.active->claim > opcon.active->contract )
     {
          /* The active dispatch's contract has expired. */
          /* it is time to switch in the next dispatch   */

          /* Check if the current dispatch is actually   */
          /* an Executor operation, in which case we     */
          /* don't modify the claim */

          if ( opcon.active->status < DS_EXEC_IDLE )
               /* We also lock the claim in at the contract   */
               /* this ensures if our schedule re-starts,     */
               /* everyone gets just one slot until tomorrow  */
               opcon.active->claim = opcon.active->contract;
          
          /* If we get to the end of the schedule, we    */
          /* just repeat the schedule until the day is   */
          /* over. Basically groundhog day.              */

          opsec_xdm
          (
               opcon.active->nx_sched ?
               opcon.active->nx_sched :
               opcon.sched 
          );

     }

     if ( opcon.active->transact->mark )
          checktx ( );

     free_xs ( &opcon.xs_in );
     return;
          

}


static void checktx ( void )
{
     /* We are still protected by the XS$ mutex. So we    */
     /* are thread safe, and will use it to our advantage */
     /* to check the transaction account and the dispatch */
     /* commit target */

     /* If the semaphore is still where we set it, AND    */
     /* there have been no commits to the block, it means */
     /* that our transaction has not been interrupted     */

     /* If we are interrupting someone who interrupts us, */
     /* we can expect the semaphore to be greater, but    */
     /* the commit might still be at our target if they   */
     /* haven't commited yet.  */

     if
     (
          ( opcon.active->super->cisec.account->com ==
            opcon.active->transact->ctarget
          ) &&
          ( opcon.active->super->cisec.account->act ==
            opcon.active->transact->amark
          )
     )
          /* We're good! */
          return;
     


     /* Looks like we lost of the transaction, need to    */
     /* roll back. This will not return, so free xs first */
     free_xs ( &opcon.xs_in );
     opsec_tx_rollback
     (
          opcon.active->transact,
          opcon.active->super->cisec.account,
          opcon.active->cf
     );

     return;  /* "return" */
}

     

          
