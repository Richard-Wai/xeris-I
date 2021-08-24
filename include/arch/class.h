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
 *  <arch/class.h>
 *
 *  AutomaticaArchitecture classification and
 *  grouping defintions
 *
 */

#ifndef __XERIS_ARCH_CLASS_H
#define __XERIS_ARCH_CLASS_H

/* Global AVR architecture family class */
#if defined(__XERIS_BUILD_ARCH_avr5)
#ifndef __XERIS_ARCH_AVR
#define __XERIS_ARCH_AVR
#endif
#endif


/* Global architecture type class */

/* Harvard */
#if defined(__XERIS_ARCH_AVR)
#ifndef __XERIS_ARCH_HARVARD
#define __XERIS_ARCH_HARVARD
#endif
#endif


#endif
