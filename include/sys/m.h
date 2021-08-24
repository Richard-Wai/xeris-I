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
 *  m.h
 *
 *  Target-specific Low-level defitionions and functions/macros.
 *  For core OS development and drivers
 */

#ifndef __XERIS_SYS_M_H
#define __XERIS_SYS_M_H

#ifdef __XERIS_BUILD_ARCH_avr5
#include <arch/avr5/sys/m/m_avr5.h>
#else
#error "Target architecture not supported"
#endif

#endif
