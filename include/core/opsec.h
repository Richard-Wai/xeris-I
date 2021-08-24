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
 *  OPSEC
 *  Operations Secretariat
 *
 *  <core/opsec.h>
 *  Definitions and Core Data Structures
 */

#ifndef __XERIS_CORE_OPSEC_H
#define __XERIS_CORE_OPSEC_H

#include <xeris.h>
#include <sys/m.h>
#include <core/std/dispatch.h>
#include <core/atria.h>
#include <core/siplex.h>

/* Interrupt Configuration File */

/* opcon_int shall always be declared or referenced as volatile */

/* Note the stack base targets are not volatile because   */
/* These pointers are never de-referenced. They are there */
/* specifically to hold addresses (on the stack)          */

struct opsec_opcon_int
{
     void                        * sbase;      /* Interrupt stack base   */
     void                        * slim;       /* Stack limit line       */
     void                        * smark;      /* stack mark             */

     uint8_t                       sigpre;     /* premption signal       */
     uint8_t                       throtl;     /* Nesting throttle flag  */
     uint8_t                       kilmsk;     /* Kill mask */

     volatile struct siplex_imsg * msg_q;      /* Message queue          */
     volatile struct siplex_imsg * msg_max;    /* Queue end              */

     uint64_t                      sisr;       /* SISR Table Reference   */
     uint16_t                      dtimer;     /* Designated timer       */
};

/* Operational Control File */

/* opcon shall always be delcared and referenced as volatile */

struct opsec_opcon
{
     volatile dispatch * mission;   /* Top of dispatch binder */
     volatile dispatch * active;    /* Active Dispatch        */
     volatile dispatch * sched;     /* Top of schedule        */

     uint8_t             xs_in;     /* XS$ Executing          */
     uint16_t            xs_tick;   /* XS$ Tick               */
     uint16_t            p_tick;    /* Preemption Tick        */
     uint8_t             p_soft;    /* Soft preemption enable */

     uint16_t            sslots;    /* Slots per "day"        */
     uint16_t            prothr;    /* Promotion Threshold    */
     uint16_t            dlmark;    /* Deadline mark          */
     uint16_t            dllimt;    /* Deadline bump line     */
     uint8_t             dlbump;    /* Deadline bump ammount  */
     uint8_t             climit;    /* Critical limit ammount */

     volatile struct
     opsec_opcon_int   * intr;      /* Interrupt handling     */

     uint16_t            telcom;    /* Telemetry Commissioner */
};


#endif
