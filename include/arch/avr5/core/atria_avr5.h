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
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authroity
 *
 *  include/core/atria.h
 *  Definitions, Core Data Structures and Prototypes
 */

// for AVR_5 only! This file should be include from <xeris.h> ONLY!

#ifndef __XERIS_CORE_ATRIA_H
#error "File shall be included from include/atria.h ONLY."
#endif

#ifndef __XERIS_CORE_ATRIA_H_ARCH_avr5_H
#define __XERIS_CORE_ATRIA_H_ARCH_avr5_H 0.1

/* check for compatability with __flash and __memx */
#if !defined(__MEMX) || !defined(__FLASH)
#warning "AVR named spaces not supported by this version of gcc."
#warning "Build may fail. Please upgrade GCC to 6.1 or later."
#endif

#include <xeris.h>
#include <sys/types.h>


/* Secretariat Operational Certificate */
struct atria_soc
{
     uint16_t       sid;             /* Secretariat ID                 */
     uint16_t       (*dd) (xdcf *);  /* Dispatch Desk                  */

     uint16_t       bpri;            /* Base priority                  */
     uint16_t       dsec;            /* Downstream Security Level      */
     uint16_t       usec;            /* Upstream Security Level        */

     volatile xta * account;         /* Registered Transaction Account */
};

/* Secretariat Interrupt Service Registration */
struct atria_sisr
{
     uint8_t   flags;           /* ISR flags  */     
     uint16_t  (*isr) (void);   /* Actual ISR */
     uint16_t  office;          /* SID of connected Secretariat */     
};

/* Secretariat Registration File              */
/* Refer to ATRIA% Operational Specification  */
/* xeris!atria%opspec*                        */
struct atria_srf
{
     struct
     {
          const __memx void * top;
          const __memx void * bot;
     } text;
     
     struct
     {
          const __memx void * ofs;
          void              * lda;
          uint16_t            siz;          
     } data;

     struct
     {
          void              * lda;
          uint16_t            siz;          
     } bss;

     struct atria_soc         soc;  /* Operational Certificate  */
     
};




#endif
