/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  xeris_avr5.h
 *
 *  XERIS Standard Submit Proceedure Definition
 */

/* For AVR, the SUBMITPOINT is one past the last vector */

#ifndef __XERIS_H_avr5_H
#define __XERIS_H_avr5_H

#include <sys/types.h>

#if defined (__XERIS_BUILD_DEV_atmega328p)

#define __XERIS_SUBMITPOINT 0x0034

#define xeris(SID) ((void (*)(uint16_t))(0x0034)) (SID)

#else
#error "AVR5 Device Type not supported."
#endif

#endif
