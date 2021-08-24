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
 *  OPSEC%
 *  Operational Secretariat
 *
 *  <opsec/sys.h>
 *  Low-level System Proceedures
 */

#include <xeris.h>

/* kill stack and return to terminal return */
extern void sys_terminate ( void * ret, void * mark  );

/* kick to ATRIA% Dispatch Desk */
extern void sys_atria ( xdcf * );

/* kick direct to FLATLINE% dispatch desk */
extern void sys_flatline ( xdcf * );

/* first-boot init and relay to FLATLINE%SYSBOOT$ */
extern void sys_boot ( void );

/* Simple fast, light mem copy */
extern void sys_memcpy ( void * src, void * dst, uint8_t size );
