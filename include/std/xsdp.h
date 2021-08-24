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
 *  XERIS Standard Dispatch Procedure
 *  Standard Definitions and Data Structures
 *
 *  <std/xsdp.h>
 */

#ifndef __XERIS_STD_XSDP_H
#define __XERIS_STD_XSDP_H

#if defined(__XERIS_BUILD_ARCH_avr5)   /* AVR5 */
#include <arch/avr5/std/xsdp_avr5.h>
#else
#error "Target architecture not supported."
#endif

/* Dispatch Shot Card */
typedef struct
{
     volatile uint8_t    status;          /* Card status           */
              uint8_t    intent;          /* Shot intent           */

              uint16_t   client;          /* SID of originator     */
              uint16_t   contractor;      /* SID of first receiver */

              uint16_t   permit;          /* Special permit number */

              xdcf       master;          /* Master case file      */
     
} shotcard;


/* Shotcard status codes */
#define SHOT_UNL  0x00         /* Ejected            */
#define SHOT_RDY  0x01         /* Ready for loading  */
#define SHOT_HOT  0x02         /* Shot ready to fire */
#define SHOT_FIR  0x03         /* Shot fired         */
#define SHOT_FRE  0x04         /* Shot recycled/free */

/* Shotcard intent codes */
#define SHOT_CMD  0x00         /* Single command     */
#define SHOT_TRA  0x01         /* Transaction        */
#define SHOT_CYC  0x02         /* Repetative         */
#define SHOT_EXL  0x03         /* Exclusive          */
#define SHOT_PER  0x04         /* Permit             */
#define SHOT_PRV  0x10         /* Private Qualifier  */

/* Ticket Stubs */
/* Standard Tickets (00-0f) */
#define XST_SRQ   0x00   /* Service Request        (xst_srq)  */
#define XST_MSG   0x01   /* Standard Message       (xst_msg)  */

/* Report Tickets (10-1f) */
#define XST_RPT   0x10   /* Standard Report        (xst_rpt)  */
#define XST_CPL   0x11   /* Completion Report      (xst_rpt)  */
#define XST_PAR   0x12   /* Partial Completion     (xst_rpt)  */
#define XST_UNA   0x13   /* Unable                 (xst_rpt)  */
#define XST_APR   0x14   /* Request Approved       (xst_rpt)  */
#define XST_DNY   0x15   /* Request Denied         (xst_rpt)  */
#define XST_ACP   0x16   /* Dispatch Accepted      (xst_rpt)  */
#define XST_RJT   0x17   /* Dispatch Rejected      (xst_rpt)  */
#define XST_TLM   0x18   /* Telemetry Report       (xst_tlm)  */
#define XST_EVT   0x19   /* Event Ticket           (xst_evt)  */
#define XST_EXC   0x1e   /* Exception Raised       (xst_tlm)  */
#define XST_SYS   0x1f   /* System Intervention    (xst_rpt)  */

/* Packet Tickets (20-2f) */
#define XST_PCTL  0x20   /* Control Packet         (xst_pctl) */
#define XST_PSTR  0x21   /* String Packet (808)    (xst_p808) */
#define XST_P808  0x22   /* 8 x 8-bit packet       (xst_p808) */
#define XST_P416  0x23   /* 3 x 16-bit packet      (xst_p416) */
#define XST_P232  0x24   /* 1 x 8-bit packet       (xst_p232) */
#define XST_P164  0x25   /* 1 x 16-bit packet      (xst_p164) */

/* Track Tickets (30-3f) */
#define XST_TRAK  0x30   /* Track Waybill          (xst_trak) */
#define XST_TLEA  0x31   /* Track Lease Ticket     (xst_tlea) */
#define XST_TADM  0x32   /* Track Admin Ticket     (xst_tadm) */
#define XST_TMSG  0x33   /* Message Truck          (xst_tmsg) */
#define XST_TSYM  0x34   /* Symbol Truck           (xst_tsym) */
#define XST_TPAC  0x35   /* Packet Truck           (xst_tpac) */
#define XST_TMAN  0x36   /* Manifest Truck         (xst_tman) */
#define XST_TTOP  0x37   /* Ticket Truck Top       (xst_ttop) */
#define XST_TBOT  0x38   /* Ticket Truck Bottom    (xst_tbot) */
#define XST_TRUK  0x39   /* Bulk  Truck            (xst_truk) */
#define XST_TPR0  0x3a   /* Private Truck 0        (xst_truk) */
#define XST_TPR1  0x3b   /* Private Truck 1        (xst_truk) */
#define XST_TPR2  0x3c   /* Private Truck 2        (xst_truk) */
#define XST_TPR3  0x3d   /* Private Truck 3        (xst_truk) */
#define XST_TPR4  0x3e   /* Private Truck 4        (xst_truk) */
#define XST_TPR5  0x3f   /* Private Truck 5        (xst_truk) */


/* Case Board Configurations */

#endif
