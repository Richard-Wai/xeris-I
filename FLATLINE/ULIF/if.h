/*
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  XERIS CORE
 *
 *  FLATLINE%ULIF$
 *  Umbilical Link Interface Operator
 *
 *  Driver Abstraction Interface
 *
 */

#ifndef __FLATLINE_ULIF_IF_H
#define __FLATLINE_ULIF_IF_H

#include <sys/types.h>

extern uint16_t      ulif_if_init   ( void );
extern void          ulif_if_rels   ( void );
extern void          ulif_if_txchar ( unsigned char c );
extern unsigned char ulif_if_rxchar ( void );

#endif
