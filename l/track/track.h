/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED
 */

/*
 *  XERIS Logistics Group
 *
 *  l/track%
 *  Track Registrar
 *
 *  GENERIC Release 1
 *  Version 1.0
 *
 *  track.c
 *  Dispatch Desk
 *
 */

#ifndef __L_TRACK_H
#define __L_TRACK_H

#ifndef __CFG_L_TRACK_TTRACKS
#define __CFG_L_TRACK_TTRACKS 6
#endif

/* Our own SID */
#define XSEC_L_TRACK 3000

/* Init desk - must be called from */
/* Operations Group (SID < 000f)   */
#define DESK_L_TRACK_INIT 0xffff

/* Internal Transaction Desks codes */
#define TX_PUSH    0x0000
#define TX_EXEC    0x0001

/* Transaction Opcodes */
#define TX_PULL    0x00
#define TX_NEW     0x01
#define TX_TERM    0x02
#define TX_UPDATE  0x03
#define TX_INIT    0xff

/* Result codes */
#define TX_OK      0xab
#define TX_FAIL    0xcd

/* For readability */
#define TRANSACTION XSEC_L_TRACK

/* Total tracks */
#define TTRACKS 8


/* Track Registration Files */
struct trf
{
     uint8_t        idx;
     struct trf  * next;

     uint16_t  dispatch;
     
     struct
     {
          uint16_t sid;
          uint16_t dsk;
     } adm;

     struct
     {
          uint16_t sid;
          uint16_t dsk;
     } agt;
};
     

/* Transaction Cards */
struct txcard
{
     struct txcard * next;      /* Next in queue  */
     
     uint8_t op;                /* Operation Code */
     uint8_t result;            /* Result of op   */
     struct trf session;        /* Session file   */
};

/* Transaction Account */
struct txaccnt
{
     struct txcard * q_first;
     struct txcard * q_last;

     struct trf    * freelist;
};


/* Globals */
extern struct trf tracks[];
extern struct txaccnt tx_a, tx_b;

extern uint8_t is_init;

extern xta transact;

/* Operators */
extern uint16_t op_tx    ( xdcf * );

#endif
