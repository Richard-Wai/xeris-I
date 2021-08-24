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
 *  FLATLINE%
 *
 *  <flatline/flatline.h>
 *  Internal Direct access to operators
 */

#ifndef __FLATLINE_FLATLINE_H
#define __FLATLINE_FLATLINE_H

#include <xeris.h>
#include <sys/types.h>

/* DD */
extern uint16_t flatline ( xdcf * );

/* ULIF$ */
extern uint16_t flatline_ulif ( xdcf * );

/* SYSBOOT$ */
extern void flatline_sysboot ( xdcf * );

/* OMNI$ */
extern uint16_t flatline_omni ( xdcf * );

#endif     
