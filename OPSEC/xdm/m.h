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
 *  XFM$
 *  Executive Fleet Marshal
 *
 *  opsec/xfm/m.h
 *  Machine-Dependent Prodedures
 */

#include <xeris.h>
#include <core/std/dispatch.h>

/* Execute Marshal */
extern void opsec_xdm_exec
(
     volatile struct dispatch_zone             * outmar,
     volatile struct dispatch_zone             * inmar,
     volatile dispatch              * volatile * actd,
     volatile dispatch                         * chkin
);

/* Procedure is to be implemented in assembly */

