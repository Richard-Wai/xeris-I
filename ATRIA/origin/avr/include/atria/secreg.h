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
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  ATRIA/ORIGIN (AVR)
 *
 *  SECREG$
 *  Secretariat Registrar 
 *
 *  <atria/secreg.h>
 */


#ifndef __ATRIA_SECREG_H
#define __ATRIA_SECREG_H

#include <xeris.h>
#include <sys/types.h>
#include <core/atria.h>

/* Lockdown List */
#define ATRIA_LDL_LEN 10

/* Main SECREG$ operator prototype */
extern uint16_t atria_secreg ( xdcf * );

/* Assistants */
extern uint8_t atria_secreg_pullsrf ( uint16_t, struct atria_srf * );
extern uint8_t atria_secreg_pullsoc ( uint16_t, struct atria_soc * );

extern uint16_t atria_secreg_nextsid ( uint16_t mark );

extern void atria_secreg_svc_opcert ( xdcf * );


#endif
