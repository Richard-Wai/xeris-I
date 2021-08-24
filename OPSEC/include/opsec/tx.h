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
 *  TX$
 *  Transaction Manager
 *
 *  <opsec/tx.h>
 *  Prototypes
 */

#ifndef __OPSEC_TX_H
#define __OPSEC_TX_H

#include <xeris.h>
#include <core/std/dispatch.h>

/* Machine-dependent (asm) */
extern void opsec_tx_event
( volatile struct dispatch_transact *, volatile xta *, xdcf * );

extern void opsec_tx_rollback
( volatile struct dispatch_transact *, volatile xta *, xdcf * );

#endif
