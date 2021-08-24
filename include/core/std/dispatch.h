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
 *  CORE STANDARDS
 *
 *  <core/std/dispatch.h>
 *  Dispatch Package and Operational File Definitions
 */

#ifndef __XERIS_CORE_STD_DISPATCH_H
#define __XERIS_CORE_STD_DISPATCH_H

#ifndef __ASSEMBLER__

/* Target-specific */
#if defined (__XERIS_BUILD_ARCH_avr5)     /* AVR5 */
#include <arch/avr5/core/std/dispatch_avr5.h>
#else
#error "Target architecture not supported."
#endif

#include <std/xsdp.h>

/* Dispatch Definition */

/* Dispatch files should always be declared volatile */

/* Note that the case file itself is not marked as volatile. This   */
/* is because the case file shall never be changed while in any     */
/* given subprogram by anything else except that subprogram.        */
/* It should be considered illegal for any case file to be volatile */

typedef struct dispatch_s
{
     uint8_t                 status;    /* Dispatch Status        */

     uint16_t                priority;  /* Priority               */
     uint16_t                deadline;  /* Deadline (work days)   */
     uint8_t                 contract;  /* Slots promised per day */
     uint8_t                 claim;     /* Slots used today       */
     uint16_t                sideline;  /* Sideline counter       */

     xdcf                  * cf;        /* Case File              */
     
     volatile struct
     dispatch_zone         * zone;      /* Zone file              */
     
     volatile struct
     dispatch_super        * super;     /* Supervisor File        */

     volatile struct
     dispatch_s            * nx_binder; /* Next in binder         */
     volatile struct
     dispatch_s            * nx_sched;  /* Next in schedule       */

     volatile struct
     dispatch_transact     * transact;  /* Transaction File       */

} dispatch;

#endif

/* Standard Code Definitions */

/* Zone File Status Codes */
#define DZ_ZONE_RDY   0x00    /* Zone OK            */
#define DZ_ZONE_WRN   0x01    /* Excursion Warning  */
#define DZ_ZONE_LEX   0x02    /* Local Excursion    */
#define DZ_ZONE_ZEX   0xff    /* Zone Excursion     */


/* Dispatch Status Codes */
/* Operational codes */
#define DS_SCH        0x00    /* Scheduled for execution   */
#define DS_EXE        0x01    /* Selected for execution    */
#define DS_RLS        0x02    /* Dispatch released         */

/* Scheduling codes */
/* The following are in the order in which they are schedule */
/* DS_DEX has highest priority */
#define DS_DEX        0x03    /* Deadline Expadite         */
#define DS_DAP        0x04    /* Deadline approach         */
#define DS_PRO        0x05    /* Promoted                  */
#define DS_SLK        0x06    /* Slack dispatch (normal)   */

/* The low nibble matches the regular dispatch codes above */
/* They are schedule according to the same rules. OPSEC%OS$*/
/* maskes the high nibble and treats them identically      */

#define DS_EXEC_EMRG  0x13    /* Executor emergency        */
#define DS_EXEC_OP    0x14    /* Executor is active        */
#define DS_EXEC_IDLE  0x15    /* feed queues are empty     */

#define DS_BLK        0xe0    /* blocked */
#define DS_OOS        0xfc    /* Out of Service            */
#define DS_HLT        0xfe    /* Halted                    */
#define DS_TRM        0xff    /* Terminated                */



#endif
