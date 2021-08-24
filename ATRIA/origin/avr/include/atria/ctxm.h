/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  All Rights Reserved
 */

/*
 *  XERIS CORE
 *
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  ATRIA/ORIGIN (AVR)
 *
 *  CTXM$
 *  Context Manager 
 *
 *  <atria/ctxm.h>
 */

#ifndef __ATRIA_CTXM_H
#define __ATRIA_CTXM_H

#include <xeris.h>

/* Primary Operator */
extern uint16_t atria_ctxm ( xdcf * );

/* Secondary operators */
extern void atria_ctxm_initgen ( xdcf * );

/* Assistants */
extern void * atria_ctxm_ld0_block
     ( const __memx void * source, void * target, uint16_t run );
extern void * atria_ctxm_ld0_bsclr
     ( void * target, uint16_t run );

#endif
