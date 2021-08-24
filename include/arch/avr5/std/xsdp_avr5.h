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
 *  XERIS Standard Dispatch Proceedure
 *  Standard Definitions and Data Structures
 *
 *  For avr5 architecture
 *  <std/xsdp.h>
 */


#ifndef __XERIS_STD_XSDP_H
#error "File shall be included from <std/xsdp.h> ONLY."
#endif

#ifndef __XERIS_STD_XSDP_H_ARCH_avr5_H
#define __XERIS_STD_XSDP_H_ARCH_avr5_H 1

#include <sys/types.h>
#include <std/xsts.h>

/* Case Board Structre */
struct xsdp_case_board_s
{
     void     * p;  /* Pointer to top of board space */
     uint16_t   s;  /* Size of work area (bytes)     */
     
};

/* Work Order Structrue */
struct xsdp_work_order_s
{
     uint16_t   d;    /* Desk                        */
     uint8_t    s;    /* Ticket stub (type info)     */
     xst_std    t;    /* Ticket                      */
};

/* Stamp */
struct xsdp_stamp_s
{
     uint16_t   sid;  /* Upstream secretariat      */
     uint16_t   dsk;  /* Upstream desk             */
     uint16_t   sec;  /* Downstream security level */
};

/* Dispatch Case file */
typedef struct
{
     volatile struct xsdp_case_board_s  b;    /* Work Area */
     volatile struct xsdp_work_order_s  w;    /* Work Order */

     volatile struct xsdp_stamp_s       s;    /* Stamp/Secruity/Signature */

} xdcf;

/* XERIS Transaction Account */
/* Components here are intrincically volatile because, like */
/* the case file, treating those components as non volatile */
/* is always a bad idea */
typedef struct
{
     void     * volatile current;    /* Current Account      */
     void     * volatile event;      /* Event Slip           */

     uint16_t   act;                 /* Active Slips         */
     uint16_t   com;                 /* Block commits        */
} xta;


#endif
