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
 *  <siplex/siplex.h>
 *  Globals
 *
 */

#ifndef SIPLEX_H
#define SIPLEX_H

#include <xeris.h>
#include <xeris/siplex.h>
#include <core/std/dispatch.h>
#include <core/flatline.h>
#include <core/atria.h>
#include <core/opsec.h>
#include <core/siplex.h>

/* Configuration Defaults */
#define SIPLEX_LANES        2      /* maximum registered dispatches  */
#define SIPLEX_SHOTQS       4      /* shot queues. minimum is 4.     */
#define SIPLEX_FEEDERS      (SIPLEX_LANES * 4)
#define SIPLEX_SHOTS        10     /* Public shotcards               */
#define SIPLEX_SHELLS       5      /* empty chains for private shots */

#define SIPLEX_ZONE_THRESH  20     /* Default zone threshold         */
#define SIPLEX_ZONE_BOARD   30     /* Default zone guard/board size  */

#define SIPLEX_INTR_ZONE    200    /* Interrupt stack size           */
#define SIPLEX_INTR_GUARD   20     /* Interrupt stack guard size     */
#define SIPLEX_INTR_QUEUE   10     /* Interrupt Message Queue Length */

/* Queue Afinitiy Codes */
#define Q_CMD  SHOT_CMD            /* Command queue          */
#define Q_TRA  SHOT_TRA            /* Transaction queue      */
#define Q_CYC  SHOT_CYC            /* Cyclic queue           */
#define Q_EXL  SHOT_EXL            /* Exclusive queue        */
#define Q_PER  SHOT_PER            /* Permit-only queue      */

#define Q_FRE  0x30                /* Chain recycle queue    */
#define Q_RSV  0x31                /* Reserved chain recycle */
#define Q_INT  0x32                /* Interrupt dispatch     */

/* Mainline queue indexes */
#define INIT_Q_CMD       0
#define INIT_Q_CYC       1
#define INIT_Q_TRA       2
#define INIT_Q_INT       3

#if (SIPLEX_SHOTQS < 4)
#error "SIPLEX/STARLINE (AVR): Minimum 4 Shot Queues required"
#endif

/* OPCON Configuration Defaults */
/* This was originally a customizable load-at-boot config  */
/* block. This has been replaced with compiled in defaults */
/* which can then be changed by the appropriate calls to   */
/* SIPLEX% */

#define OPCON_OPSTRAP_P_SOFT 0x00    /* opcon.p_soft: Soft preemption     */
#define OPCON_OPSTRAP_SSLOTS 0x2f    /* opcon.sslots: Slots per day       */
#define OPCON_OPSTRAP_PROTHR 0x05    /* opcon.prothr: Promotion threshold */
#define OPCON_OPSTRAP_DLMARK 0x10    /* opcon.dlmark: Deadline threshold  */
#define OPCON_OPSTRAP_DLLIMT 0x04    /* opcon.dllimt: Deadline limit      */
#define OPCON_OPSTRAP_DLBUMP 0x10    /* opcon.dlbump: Deadline bump       */


/* Note that, SIPLEX is basically the nexus of task-switching for  */
/* the XERIS system, so pretty much every global variable shall be */
/* considered as volatile. */

struct shotchain
{
     volatile        shotcard  * shot;
     volatile struct shotchain * next;
};


/* shotqueues shall always be referenced/declared as volatile */
struct shotqueue
{
     uint8_t                     afn;   /* Queue Afinity */
     volatile struct shotchain * head;  /* Top of chain  */
     volatile struct shotchain * tail;  /* Bottom of chain */
};

/* feedline shall always be referenced/declared as volatile */
struct feedline
{
     volatile struct shotqueue * feeder;
     volatile struct feedline  * next;
};

/* Dispatch Lane Descriptor File */
/* lane shall always be referenced/declared as volatile */
struct lane
{
     volatile struct feedline   * feed;
     volatile struct shotqueue  * recycle;
     volatile struct shotqueue  * eject;
     volatile        dispatch   * commission;
};
     
/* SIPLEX General Configruation File */
/* This shall always be referenced/declared as volatile */
struct sicon
{
     uint16_t def_deadline;
     uint16_t def_contract;

     uint16_t lvl_lease;
     uint16_t lvl_priv;

     uint16_t sys_sec;  /* Minimum secuirty level for sys$  */
};


/* Operational data */

/* Mainline */
/* The following are the actual, physical system allocations */
/* for all lanes */
extern volatile struct lane              lanes[];
extern volatile struct dispatch_zone     zones[];
extern volatile struct dispatch_super    supers[];
extern volatile struct dispatch_transact transacts[];
extern volatile        dispatch          commissions[];
extern volatile        shotcard          shots[];
extern volatile struct shotqueue         queues[];
extern volatile struct shotchain         sabots[];
extern volatile struct feedline          feeders[];

/* Intrrupt Manager */
extern volatile struct dispatch_zone  intzone;
extern volatile struct siplex_imsg    intmsg[2][SIPLEX_INTR_QUEUE];

/* Link pools */
extern volatile struct shotqueue magazine;
extern volatile struct shotqueue shells;

/* Depreciated */
/* Primer chain (init_primer.s) */
//extern const __flash shotcard * const __flash primer[];

/* OPSEC Configuration File */
/* **very volatile** */
extern volatile struct opsec_opcon * opcon;

/* SIPLEX General configuration */
extern volatile struct sicon sicon;

/* Dispatch Desk */
extern uint16_t siplex ( xdcf * );

/* Public Desks */
extern uint16_t subdesk ( xdcf * );   /* SUB$ */
extern uint16_t sysdesk ( xdcf * );   /* SYS$ */

/* Internal Operators */
extern uint16_t           siplex_init  ( xdcf * );
extern volatile struct shotqueue * siplex_route
( uint8_t intent, uint16_t permit );


/* Executor Division */
extern void executor ( volatile struct lane * job );
extern void bootstrap ( void );

extern void exec_boot ( void * base );
extern void exec_seed ( volatile struct lane * lane );

/* the following do not return */
extern void exec_crank ( volatile struct lane * job );
extern void exec_reset ( void );



/* Assistants */
/* rom load should be depreciated */
/* extern void rom_load ( const __memx void * ld, void * st, uint16_t siz ); */
extern void mem_copy ( void * original, void * copy, uint16_t siz );
extern void int_ena  ( void );
extern void int_dis  ( void );


/* Atomic operations */
extern volatile struct shotchain * pop_shot  ( volatile struct shotqueue * );
extern void put_shot
( volatile struct shotqueue *, volatile struct shotchain * );

extern void int_swmq
( volatile struct siplex_imsg * volatile * old_mark,
  volatile struct siplex_imsg            * new_mark,
  volatile struct siplex_imsg * volatile * old_max,
  volatile struct siplex_imsg            * new_max );

extern uint8_t  a8        ( volatile void * target, uint8_t val );
extern uint16_t a16       ( volatile void * target, uint16_t val );

#endif
