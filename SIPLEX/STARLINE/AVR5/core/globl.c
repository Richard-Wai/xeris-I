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
 *  Global Variable Definitions
 *
 */

#include <xeris.h>
#include <core/opsec.h>
#include <siplex/siplex.h>

/* Dispatch Lanes */
volatile struct lane                lanes[SIPLEX_LANES];
volatile struct dispatch_zone       zones[SIPLEX_LANES];
volatile struct dispatch_super      supers[SIPLEX_LANES];
volatile struct dispatch_transact   transacts[SIPLEX_LANES];
volatile        dispatch            commissions[SIPLEX_LANES];


/* Shot Queues */
volatile        shotcard            shots[SIPLEX_SHOTS];
volatile struct shotchain           sabots[SIPLEX_SHOTS + SIPLEX_SHELLS];
volatile struct shotqueue           queues[SIPLEX_SHOTQS];


/* Feeders */
volatile struct feedline            feeders[SIPLEX_FEEDERS];

/* Interrupt handling */
volatile struct dispatch_zone       intzone;
volatile struct siplex_imsg         intmsg[2][SIPLEX_INTR_QUEUE];

/* Chain pools */
volatile struct shotqueue magazine;
volatile struct shotqueue shells;


/* OPCON */
volatile struct opsec_opcon * opcon;

/* SIPLEX General configuration */

volatile struct sicon sicon =
{
     .def_deadline = 0xffff,
     .def_contract = 4,

     .lvl_lease    = 0,
     .lvl_priv     = 0,

     .sys_sec      = 0xfff0
};
     

