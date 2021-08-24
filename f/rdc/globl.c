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
 *  Global Variables
 */

#include <xeris.h>
#include <xeris/strap.h>

#include <f/rdc/globl.h>

/* Transaction Handling */
volatile struct tx_ledger lega, legb;
volatile xta tx_account =
{
     .current = NULL,
     .event   = NULL,
     .act     = 0,
     .com     = 0
};


/* Dispatch handling */
struct dispatch_card ds_stack[DS_STACK_SIZE];


/* Internals */
uint16_t dctrl = X_STRAP;


