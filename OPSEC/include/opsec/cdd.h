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
 *  CDD$
 *  Central Dispatch Desk
 *
 *  opsec/include/opsec/cdd.h
 *  Internal Data Structures and Prototypes
 */

#ifndef __OPSEC_CDD_H
#define __OPSEC_CDD_H

#include <xeris.h>
#include <sys/types.h>

/* *THE* OPSEC%CDD$ Operator Prototype */
/* This is called from xeris by everyone else */
/* xeris is defined in xeris.h  */
extern void opsec_cdd ( uint16_t insec );

#endif
