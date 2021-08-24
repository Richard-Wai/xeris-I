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
 *  INTR
 *  Interrupt Service Registrat 
 *
 *  <atria/intr.h>
 */

#ifndef ATRIA_INTR_H
#define ATRIA_INTR_H

#include <xeris.h>
#include <core/atria.h>

#if defined(__XERIS_BUILD_DEV_atmega328p)
#define INTR_MAXISR 25
#else
#error "ATRIA/ORIGIN (AVR): Device not supported"
#endif

/* actual SISR registry */
extern void atria_intr_sisr ( uint64_t * sisr );

extern uint16_t atria_intr ( xdcf * );


#endif
