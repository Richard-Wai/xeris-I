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
 *  XERIS Standardized Ticket Specifications
 *  std/xsts.h
 */

#ifndef __XERIS_STD_XSTS_H
#define __XERIS_STD_XSTS_H

#include <sys/types.h>

/* xst_srq - Service Request Ticket */
/* xeris?xsts*4/0 */
typedef struct
{
     uint16_t rqc;

     union
     {
          uint8_t  p08[8];
          uint16_t p16[4];
          uint32_t p32[2];
          uint64_t p64;
     } prm;

} xst_srq;

/* xst_msg - Standard Message Format */
/* xeris?xsts*4/1 */
typedef struct
{
     uint16_t ref;      /* Reference */

     union
     {
          uint8_t  a08[8];     /* Argument (8 bit)  */
          uint16_t a16[4];     /* Argument (16 bit) */
          uint32_t a32[2];     /* Argument (32 bit) */
          uint64_t a64;        /* Argument (64 bit) */
     } arg;

} xst_msg;


/* xst_rpt - Standard Report Ticket */
/* xeris?xsts*4/10 */
typedef struct
{
     uint16_t spec;
     uint16_t code;

     uint8_t cond;
     uint8_t note;

     union
     {
          uint8_t  adi08[4];
          uint16_t adi16[2];
          uint32_t adi32;
     } adi;

} xst_rpt;
     

/* xst_tlm - Telemetry Ticket */
/* xeris?xsts*4/3 */
typedef struct
{
     uint16_t spec;
     uint16_t code;

     uint16_t sec;              /* Originating Secretariat */

     union
     {
          uint8_t  dgm08[4];
          uint16_t dgm16[2];
          uint32_t dgm32;
     } dgm;                     /* DGM */
     
} xst_tlm;

/* xst_evt - Event Ticket */
/* xeris?xsts*4/4 */
typedef struct
{
     uint16_t  sid;             /* Handler SID      */
     uint16_t  dsk;             /* Handler Desk     */
     uint16_t  ref;             /* Event Identifier */
     uint32_t  seq;             /* Sequence number  */
     
} xst_evt;


/* xst_chr - Timespan Ticket */
/* xeris!xsts*4/5 */
typedef struct
{
     uint8_t   epoch;
     uint8_t   expon;
     uint16_t  span;

} xst_chr;

/* xst_pctl - Control Packet */
/* xeris?xsts*4/20 */
typedef struct
{
     uint16_t seq;            /* Packet Sequence Number */
     uint16_t ctl;            /* Control Code */
     uint16_t fir;            /* Sequence Reference (First) */
     uint16_t las;            /* Sequence Reference (Last) */
     uint16_t ref;            /* Reference Tag */

} xst_pctl;

/* xst_pswi - Switch packet */

/* xst_pstr - Character String Payload */
/* xeris?xsts*4/21 */
/* for AVR5 - char is 8 bits */
typedef struct
{

     uint16_t seq;            /* Packet Sequence Number */
     unsigned char pld[8];    /* Paylaod */

} xst_pstr;


/* xst_p808 - 8 x 8-bit Packet */
/* xeris?xsts*4/22 */
typedef struct
{
     uint16_t seq;            /* Packet Sequence Number */
     uint8_t pld[8];          /* Payload */

} xst_p808;


/* xst_p416 - 4 x 16-bit Payload */
/* xeris?xsts*4/23 */
typedef struct
{

     uint16_t seq;            /* Packet Sequence Number */
     uint16_t pld[4];         /* Payload */

} xst_p416;


/* xst_p232 - 2 x 32-bit */
/* xeris?xsts*4/24 */
typedef struct
{

     uint16_t seq;            /* Packet Sequence Number */
     uint32_t pld[2];         /* Payload */

} xst_p232;


/* xst_p164 - 1 x 64-bit */
/* xeris?xsts*4/15 */
typedef struct
{

     uint16_t seq;            /* Packet Sequence Number */
     uint64_t pld;            /* Payload */

} xst_p164;

/* xst_trak: Track Waybill */
/* xeris?xsts*4/20         */
typedef struct
{
     uint32_t track;          /* Track ID */
     uint16_t dispatch;       /* Dispatch ID */

     uint16_t sid;            /* Endpoint SID */
     uint16_t dsk;            /* Endpoint desk */
     
} xst_trak;

/* xst_tlea: Track Lease Ticket */
/* xstis?xsts*4/21              */
typedef struct
{
     uint16_t dispatch;       /* Dispatch ID */
     
     struct
     {
          uint16_t dsk;       /* Admin desk */
     } admin;

     struct
     {
          uint16_t sid;       /* Agent SID  */
          uint16_t dsk;       /* Agent desk */
     } agent;

     uint16_t nul;
     
} xst_tlea;

/* xst_tadm: Track Admin Ticket */
/* xeris?xsts*4/22              */
typedef struct
{
     uint32_t track;          /* Track ID */

     uint8_t  op;             /* Option */
     uint8_t  nul;

     union                    /* Comment */
     {
          struct              /* Registrant */
          {
               uint16_t sid;  /* SID */
               uint16_t dsk;  /* Desk */
          } reg;

          uint16_t dispatch;  /* Dispatch ID */

          uint32_t nul;
     } com;

} xst_tadm;

/* xst_truk: Generic Track Truck Ticket */
/* xeris?xsts*4/23                      */
typedef struct
{
     uint32_t track;          /* Track ID */
     uint16_t dispatch;       /* Dispatch ID */

     union                    /* Message */
     {
          uint8_t  m08[4];
          uint16_t m16[2];
          uint32_t m32;
     } msg;
          
} xst_truk;


/* xst_nul              */
/* xeris?xsts*3/0       */
/* 80 bits contiguous   */
typedef xst_p164 xst_nul;

/* Non-standard ticket placeholder */
typedef xst_nul xst_nst;

/* Specified Definition of xst_std as per specification (3/0) */
typedef union
{
     xst_nul nul;
     xst_nst nst;               /* Non-standard        */
     
     xst_srq srq;               /* Service Request     */
     xst_msg msg;               /* Standard Message    */
     xst_rpt rpt;               /* Standard Report     */
     xst_tlm tlm;               /* Standard Telemetry  */
     xst_evt evt;               /* Event Ticket        */
     xst_chr chr;               /* Timespan Ticket     */
     
     xst_pctl pctl;             /* Control Packet      */
     xst_pstr pstr;             /* String Packet       */
     xst_p808 p808;             /* 8 x 8 Packet        */
     xst_p416 p416;             /* 5 x 16 Packet       */
     xst_p232 p232;             /* 2 x 32 Packet       */
     xst_p164 p164;             /* 1 x 64 Packet       */

     xst_trak trak;             /* Track Waybill       */
     xst_tlea tlea;             /* Track Lease Ticket  */
     xst_tadm tadm;             /* Track Admin Ticket  */
     xst_truk truk;             /* Track Truck         */

} xst_std;


/* Global Standard Ticket Codes */

/* 4/10 xst_rpt */
/* Condition Codes */
#define RPT_NRM         0x00
#define RPT_FTL         0x01
#define RPT_WCR         0x02
#define RPT_RSB         0x03
#define RPT_ADV         0x04
#define RPT_RTY         0x05
#define RPT_CNC         0x06
#define RPT_TAL         0x07
#define RPT_OKY         0x08

/* Standard Notes */
#define RPT_NON         0x00
#define RPT_CPR         0x01
#define RPT_REF         0x02
#define RPT_STA         0x03
#define RPT_RET         0x04


/* Track Admin Options */
#define TRAK_TRM        0x00    /* Terminate Lease   */
#define TRAK_XFR        0x01    /* Transfer Lease    */
#define TRAK_RED        0x02    /* Redirect Track    */
#define TRAK_DSP        0x03    /* Update Dispatch   */

#endif
