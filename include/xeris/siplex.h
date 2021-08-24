/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  ALL RIGHTS RESERVED
 */

/*
 *  XERIS CORE
 *
 *  SIPLEX%
 *  Strategic Imparative Planning Liason Executive
 *
 *  <xeris/siplex.h>
 *  XERIS Specification Header
 */

#ifndef __XERIS_SIPLEX_H
#define __XERIS_SIPLEX_H

/* Secretariat */
#define X_SIPLEX    0x0003

/* Desks */
#define X_SIPLEX_SUB  0x0000
#define X_SIPLEX_SYS  0xffff

/* Tags */
/* sub$ */
#define X_SIPLEX_SUB_LEAS  0x0000
#define X_SIPLEX_SUB_PRIV  0x0001
#define X_SIPLEX_SUB_CANC  0x0002

/* sys$ */
#define X_SIPLEX_SYS_INIT      0x0000
#define X_SIPLEX_SYS_INT_INIT  0x0010
#define X_SIPLEX_SYS_INT_KILL  0x0011
#define X_SIPLEX_SYS_INT_TIMR  0x0012
#define X_SIPLEX_SYS_INT_SISR  0x0013
#define X_SIPLEX_SYS_INT_SUPR  0x0014

#endif
