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
 *  FLATLINE/include/internal.h
 *  Definitions and Prototypes for
 *  FLATLINE% internal operations
 */

#ifndef __FLATLINE_INTERNAL_H
#define __FLATLINE_INTERNAL_H

#include <xeris.h>
#include <sys/types.h>

/* FLATLINE Internal Helper Functions */

/* Long string processor */
extern void printstr ( unsigned char * buf, uint8_t len );

/* Integer to string conversion helper */
extern void itostr ( unsigned char *buf, uint32_t inp, uint8_t base );

/* Load strings from ROM */
extern uint8_t loadstr ( unsigned char *, uint8_t );

#endif
