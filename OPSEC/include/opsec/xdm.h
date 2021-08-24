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
 *  XDM$
 *  Executive Dispatch Marshal
 *
 *  opsec/include/opsec/xdm.h
 *  Internal Data Structures and Prototypes
 */

#ifndef __OPSEC_XDM_H
#define __OPSEC_XDM_H

#include <xeris.h>
#include <sys/types.h>
#include <core/std/dispatch.h>

/* Zone check prototype */
extern void opsec_xdm_zcheck ( volatile struct dispatch_zone * );


/* Operator Prototype */
extern void opsec_xdm ( volatile dispatch * in_marshal );


#endif
