/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  ALL RIGHTS RESERVED
 */

/*
 *  XERIS Facilities Group
 *
 *  f/rdc%
 *  Recurrent Dispatch Commission
 *
 *  <f/rdc/rdc.h>
 *  Global definitions
 */

#ifndef __F_RDC_H
#define __F_RDC_H

#include <f/rdc/globl.h>

/* Desks */
extern uint16_t sub   ( xdcf * );
extern uint16_t ctl   ( xdcf * );
extern uint16_t agt   ( xdcf * );
extern uint16_t txp   ( xdcf * );
extern uint16_t txe   ( void   );

/* Operators */
extern uint16_t init  ( xdcf * );
extern uint16_t reg   ( xdcf * );

/* Transaction Targets */
#define TX_DISPATCH  0x00
#define TX_FREE      0x10

#define TX_UNABLE    0xee       /* Indicates transaction impossible */
#define TX_COMMIT    0xff       /* Indicates transaction complete   */

/* Transaction Operations */
#define TX_PUSH      0x00
#define TX_POP       0x01
#define TX_INIT1     0xaa
#define TX_INIT2     0xbb


#endif
