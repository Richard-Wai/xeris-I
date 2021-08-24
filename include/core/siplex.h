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
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  <core/siplex.h>
 *  Universal Structures and Definitions
 *
 */

#ifndef __XERIS_CORE_SIPLEX_H
#define __XERIS_CORE_SIPLEX_H

#include <xeris.h>

struct siplex_imsg
{
     volatile uint16_t irq;         /* Interrupt Number */
     volatile uint8_t  msg;         /* Message */
};

#endif
