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
 *  <f/rdc/globl.h>
 *  Global Variables
 */

#ifndef __F_RDC_GLOBL_H
#define __F_RDC_GLOBL_H

#include <xeris.h>

/* Link Configuration */
/* Default values     */
#ifndef __XERIS_CFG_F_RDC_DS_STACK
#define __XERIS_CFG_F_RDC_DS_STACK 8
#endif

/* Import values */
#define DS_STACK_SIZE __XERIS_CFG_F_RDC_DS_STACK


/* Structure definitions */
struct dispatch_card
{
     struct dispatch_card * next;
     xst_evt                event;
};

struct dispatch_q
{
     struct dispatch_card * first;
     struct dispatch_card * last;
};
     
struct tx_card
{
     struct tx_card       * next;
     uint8_t                op;

     struct dispatch_card * dispatch;
};

struct tx_ledger
{
     struct tx_card   * e_first;     /* Transaction command queue */
     struct tx_card   * e_last;

     struct dispatch_q  d_dispatch;  /* Dispatch queues           */
     struct dispatch_q  d_free;
};

/* Transaction Accounting */
extern volatile xta    tx_account;  /* Transaction Account       */
extern volatile struct tx_ledger lega, legb;

/* Dispatch Queues Accounting  */
extern struct dispatch_card  ds_stack[];

/* Internal Globals */
extern        uint16_t  dctrl;       /* ctrl$/sync$ security      */



#endif
