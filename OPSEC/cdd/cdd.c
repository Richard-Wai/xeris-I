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
 *  XERIS CORE
 *
 *  OPSEC%
 *  Operational Secretariat
 *
 *  CDD$
 *  Central Dispatch Desk
 *
 *  opsec/cdd/cdd.c
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <opsec/opsec.h>
#include <opsec/sys.h>
#include <opsec/cdd.h>
#include <opsec/xdm.h>
#include <opsec/xs.h>
#include <opsec/super.h>
#include <opsec/tx.h>

volatile struct opsec_opcon opcon;

/* OPSEC%CDD$ Central Dispatch Desk */
void opsec_cdd ( uint16_t insec )
{
     uint16_t                popop, popdesk, retval;
     struct atria_soc        popsec;
     xdcf                  * scase;
     
     /* So here we are: in the very core of the moment  */
     /* We have been delivered here through the         */
     /* "System Call" xeris ( ); */
          
     
     /* The first thing we are going to do, is take a   */
     /* quick moment to check the Dispatch Zone File    */
     /* and stack pointer for any imminent or active    */
     /* excursion situations. If we find one,           */
     /* we will raise a system intervention immediately */
     opsec_xdm_zcheck ( opcon.active->zone );

     if ( opcon.active->zone->stat != DZ_ZONE_RDY )
     {
          /* Stack is getting out of bounds! */
          opcon.active->cf->w.d = 0;
          opcon.active->cf->w.s = XST_SYS;
          
          /* Set-up system intervention report */
          opcon.active->cf->w.t.rpt.spec = XSR_SYS;
          opcon.active->cf->w.t.rpt.note = RPT_NON;
          
          switch ( opcon.active->zone->stat )
          {
               case DZ_ZONE_WRN:
                    opcon.active->cf->w.t.rpt.code =
                         XSR_SYS_W_ZONE_THRESHOLD;
                    break;

               case DZ_ZONE_LEX:
                    opcon.active->cf->w.t.rpt.code =
                         XSR_SYS_C_LOCAL_EXCURSION;
                    break;

               default:
                    /* Game over, we need to get-out now */
                    opcon.active->cf->w.t.rpt.code =
                         XSR_SYS_F_ZONE_EXCURSION;
                    sys_terminate
                    (
                         &opcon.active->super->termret,
                         &opcon.active->super->termmrk
                    );
                    break;
          }

          return;
     }
     
     /* Stack still seems to be OK, so we can proceed   */
     /* First check if the dispatch is in an emergency  */
     /* state. If so, only the Executive Group can      */
     /* forward the dispatch */
     if ( opcon.active->status == DS_EXEC_EMRG )
     {
          /* Executive group via bgsis is 0000% - 000f% */
          if ( opcon.active->super->cisec.sid > 0x000f )
          {
               /* not executive group */
               opcon.active->cf->w.s = XST_SYS;
               opcon.active->cf->w.t.rpt.spec = XSR_SYS;
               opcon.active->cf->w.t.rpt.code = XSR_SYS_F_EMRG_LOCKDOWN;
               opcon.active->cf->w.t.rpt.cond = RPT_FTL;
               opcon.active->cf->w.t.rpt.note = RPT_NON;
               
               return;
          }
     }
     else if ( opcon.active->cf->w.s == XST_SYS )
     {
          /* XST_SYS are illegal except for the */
          /* executive group.                   */
          opcon.active->cf->w.t.rpt.spec = XSR_SYS;
          opcon.active->cf->w.t.rpt.code = XSR_SYS_P_SYS_STUB;
          opcon.active->cf->w.t.rpt.cond = RPT_CNC;
          opcon.active->cf->w.t.rpt.note = RPT_NON;
          
          return;
     }

     /* now we make sure we are not in a transaction    */
     /* event on this dispatch. If we are that is       */
     /* illegal, so we will deny the case xfer          */
     if ( opcon.active->transact->mark )
     {
          /* Track is in active event containment */
          /* only returns allowed */
          opcon.active->cf->w.s = XST_SYS;
          opcon.active->cf->w.t.rpt.spec = XSR_SYS;
          opcon.active->cf->w.t.rpt.code = XSR_SYS_P_TX_EVENT_CON;
          opcon.active->cf->w.t.rpt.cond = RPT_CNC;
          opcon.active->cf->w.t.rpt.note = RPT_NON;
          
          return;
     }
     

     /* Check if this is coming from the Executor    */
     /* with an idle signal */
     if ( opcon.active->status == DS_EXEC_IDLE )
     {
          opsec_xs ( );
          return;
     }

     /* Next, check if we have a kick to "ourselves", which */
     /* means we have a transaction event request. */
     if ( insec == opcon.active->super->cisec.sid )
     {
          /* confirmed transaction event */
          /* Make sure this secretariat has a */
          /* registered account */
          if ( !opcon.active->super->cisec.account )
          {
               /* no registered account, so we */
               /* need to deny this and say so */
               opcon.active->cf->w.s          = XST_SYS;
               opcon.active->cf->w.t.rpt.spec = XSR_SYS;
               opcon.active->cf->w.t.rpt.code = XSR_SYS_P_TX_NO_ACCNT;
               opcon.active->cf->w.t.rpt.cond = RPT_CNC;
               opcon.active->cf->w.t.rpt.note = RPT_NON;
               return;
          }
          
          /* set-up transaction file and call */
          /* the transaction manager */
          opcon.active->transact->exec =
               opcon.active->super->cisec.dd;

          /* Make sure we set-up the stamp aswell */
          opcon.active->cf->s.sid =
               opcon.active->super->cisec.sid;
          opcon.active->cf->s.sec =
               opcon.active->super->cisec.dsec;
          popdesk = opcon.active->cf->w.d;

          opsec_tx_event
          (
               opcon.active->transact,
               opcon.active->super->cisec.account,
               opcon.active->cf
          );

          /* Reset the stamp */
          opcon.active->cf->w.d = popdesk;

          
          return;
     }

     /* Cool, so we have a regular kick operation, or an  */
     /* Executor "dry run", in both cases, the next steps */
     /* are the same */

     /* We intercept telemetry submissions to 0. If */
     /* the tickets are RPT or TLM, we will set     */
     /* insec to point to telcom. If telcom is zero */
     /* the ticket types are incorrect, we will reject */
     /* out-right now. */

     /* The telemetry commissioner can be configured to */
     /* accept any ticket, but for tickets that are not */
     /* standard reports, or telemetry, the submission  */
     /* must pass ordinary security checks. Otherwise   */
     /* report tickets are converted to telemetry, and  */
     /* telemetry tickets are passed directly           */
     if ( !insec )
     {
          insec = opcon.telcom;
          
          if ( !insec )
          {
               /* No telcom defined, reject */
               opcon.active->cf->w.s          = XST_SYS;
               opcon.active->cf->w.t.rpt.spec = XSR_SYS;
               opcon.active->cf->w.t.rpt.code = XSR_SYS_D_NO_TELCOM;
               opcon.active->cf->w.t.rpt.cond = RPT_CNC;
               opcon.active->cf->w.t.rpt.note = RPT_NON;
               return;               
          }

          /* Intervene to set the official desk number   */
          /* this is one of those hard security features */
          opcon.active->cf->w.d = 0x8000;

          /* Check for RPT tickets, which we now   */
          /* convert to telemetry, and mark with   */
          /* the SID of the submitting secretariat */
          if ( opcon.active->cf->w.s == XST_RPT )
          {
               /* Convert to telemetry and automatically */
               /* tag the telemetry ticket with the SID  */
               /* of the submitting secretariat          */
               opcon.active->cf->w.s = XST_TLM;
               opcon.active->cf->w.t.tlm.sec =
                    opcon.active->cf->s.sid;
          }
     }
     

     /* Set the Check-in secretariat to the Check-out     */
     /* Check in is the "current" */
     sys_memcpy
     (
          (void *)&opcon.active->super->cisec,
          (void *)&opcon.active->super->cosec,
          sizeof ( struct atria_soc )
     );

     /* Push the current soc                  */
     /* and the supposed origin desk,         */
     /* on return, we will restore cisec      */
     sys_memcpy
     (
          (void *)(&opcon.active->super->cisec),
          (void *)(&popsec),
          sizeof ( struct atria_soc )
     );

     popop   = opcon.active->cf->w.d;

     /* Next, pull the soc for the intended secretary */
     scase = &(opcon.active->super->scase);

     /* Set-up the case board. ATRIA%SECREG$    */
     /* Expects an SOC in the case board, which */
     /* Means the case-board pointer should     */
     /* point to a SOC. We will use cisec       */
     scase->b.p = (void *)&opcon.active->super->cisec;
     scase->b.s = sizeof (struct atria_soc);

     /* Set up the Work Order                        */
     /* We also here intercept telemetry submissions */
     scase->w.d                = OP_ATRIA_SECREG;
     scase->w.s                = XST_SRQ;
     scase->w.t.srq.rqc        = ATRIA_SECREG_OPCERT;
     scase->w.t.srq.prm.p16[0] = insec ? insec : opcon.telcom;

     /* Execute */
     sys_atria ( scase );

     /* Intercept an Executor "dry run"                   */
     /* Since SIPLEX has access to opcon, the Executors   */
     /* use this dry run to load the correct SOC by proxy */
     /* And otherwise lock-in the case file correctly for */
     /* a new shot */
     if ( opcon.active->status >= DS_EXEC_OP )
          return;

     /* see of soft-preemption is enabled. If it is,        */
     /* we need to let XS decide what really happens next.. */
     if ( opcon.p_soft )
          opsec_xs ( );

     /* Check the result for succesful result */
     if ( scase->w.s != XST_CPL )
     {
          /* If we are calling the telcom, we  */
          /* Resond with a general denial      */
          if ( insec != opcon.telcom )
          {
               /* Otherwise we relay ATRIA's response */
               opcon.active->cf->w.s = scase->w.s;
               opcon.active->cf->w.t = scase->w.t;
          }
          else
          {
               opcon.active->cf->w.s          = XST_SYS;
               opcon.active->cf->w.t.rpt.spec = XSR_SYS;
               opcon.active->cf->w.t.rpt.code = XSR_SYS_D_NO_TELCOM;
               opcon.active->cf->w.t.rpt.cond = RPT_CNC;
               opcon.active->cf->w.t.rpt.note = RPT_NON;
          }

          /* Then restore the cisec */
          sys_memcpy
          (
               (void *)(&popsec),
               (void *)(&opcon.active->super->cisec),
               sizeof ( struct atria_soc )
          );
          return;
     }

     /* Cool, so now we have cisec loaded with an SOC   */
     /* We now check the cisec's upstream security with */
     /* cosec's downstream security */
     if
     (
          opcon.active->super->cisec.usec >
          opcon.active->super->cosec.dsec
     )
     {

          /* So we have a denial on the face of it */
          /* but if we have a telemetry ticket inbound */
          /* to telcom, we let it through. */

          if
          (
              !(
                    (insec == opcon.telcom) &&
                    (opcon.active->cf->w.s == XST_TLM)
               )
          )
          {
               /* No go */
               opcon.active->cf->w.s = XST_DNY;
               opcon.active->cf->w.t.rpt.spec = XSR_OP;
               opcon.active->cf->w.t.rpt.code = XSR_OP_F_ACCESS_DENIED;
               opcon.active->cf->w.t.rpt.cond = RPT_CNC;
               opcon.active->cf->w.t.rpt.note = RPT_NON;
               sys_memcpy
                    (
                         (void *)(&popsec),
                         (void *)(&opcon.active->super->cisec),
                         sizeof ( struct atria_soc )
                         );
               return;
          }
     }

     /* Note that, to get around going into a transaction */
     /* right from an executor due to a shotcard to one's */
     /* self, SIPLEX will set the "client" SID to 0 in    */
     /* such cases. That's our signal to set cosec's sid  */
     /* to the same as cisec. */
     
     opcon.active->cf->s.sid =
          opcon.active->super->cosec.sid ?
          opcon.active->super->cosec.sid :
          opcon.active->super->cisec.sid;
     
     /* We're cleared to proceed, stamp the work order    */

     opcon.active->cf->s.sec = opcon.active->super->cosec.dsec;

     /* execute */
     retval =
          opcon.active->super->cisec.dd ( opcon.active->cf );

     if ( retval )
          /* Heyo! we have a RECURSIVE kick! */
          opsec_cdd ( retval );

     /* pop old cisec back in (the one we are returning to)  */
     /* and pop the original outbound Work Order operator    */
     /* Stamp remains, to allow case file tracing. The stamp */
     /* can be whiped at any point along the path, but we    */
     /* don't touch it, unless it was a telemetry submission */
     sys_memcpy
     (
          (void *)(&popsec),
          (void *)(&opcon.active->super->cisec),
          sizeof ( struct atria_soc )
     );
     opcon.active->cf->w.d      = popop;

     if ( insec == opcon.telcom )
     {
          /* Case file tracing is disallowed when */
          /* outbound to opcon. */
          opcon.active->cf->s.sid = 0;
          opcon.active->cf->s.dsk = 0;
          opcon.active->cf->s.sec = 0;
     }
     

     /* All, done! */
     return;
}


     
     
     
        
     
     

