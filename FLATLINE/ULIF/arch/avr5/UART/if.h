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
 *  FLATLINE%ULIF$ Standard Interface Driver
 *
 *  Serial: simple fixed-config uart driver for avr5
 *
 */

#ifndef __FLATLINE_ULIF_AVR5_UART_IF_H
#define __FLATLINE_ULIF_AVR5_UART_IF_H

#include <sys/types.h>

extern uint16_t ulif_if_AVR5UART0_up ( void );
extern void ulif_if_AVR5UART0_down ( void );
extern void ulif_if_AVR5UART0_txchar
     ( register unsigned char c );
extern unsigned char ulif_if_AVR5UART0_rxchar ( void );

#endif
