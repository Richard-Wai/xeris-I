/*
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  mech.h (arch/avr5/mech/mech_avr5.h)
 *
 *  Target-specific low-level definitions
 *  and low-level functions/macros for avr5
 */

#ifndef __XERIS_H_M_avr5
#define __XERIS_H_M_avr5 0.1

#include "m_avr5_def.h"     // universal defitions
#ifndef __ASSEMBLER__        // do not include if assembly!
#include "m_avr5_ops.h"     // universal macros/functions
#endif

#ifdef __XERIS_BUILD_DEV_atmega328p
//atmega328-sepcific
#include "dev/m_avr5_atmega328p_def.h"

#else
#error "Target device not supported"
#endif

#endif
