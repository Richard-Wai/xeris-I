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
 *  INT$
 *  Interrupt Response Coordinator
 *
 *  opsec/os/os.c
 */

#include <xeris.h>
#include <core/opsec.h>
#include <core/atria.h>

#include <opsec/int.h>

volatile struct opsec_opcon_int opint;

/* Interrupt handling should be done in ASM */

