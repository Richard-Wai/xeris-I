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
 *  <siplex/int.h>
 *  Interrupt Supervisor
 *
 */

#include <xeris.h>

extern const __memx struct atria_sisr * volatile int_sisr;

extern uint16_t siplex_int_init ( xdcf * );
extern uint16_t siplex_int_kill ( xdcf * );
extern uint16_t siplex_int_timr ( xdcf * );
extern uint16_t siplex_int_sisr ( xdcf * );
extern uint16_t siplex_int_supr ( xdcf * );
