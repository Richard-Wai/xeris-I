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
 *  OS$
 *  Operational Scheduler
 *
 *  <opsec/xs.h>
 *  Internal Data Structures and Prototypes
 */

#include <xeris.h>
#include <opsec/xs.h>

/* Run at end of day to create new executive schedule */
/* Run from inside XS$ only */
extern void opsec_os ( void );
