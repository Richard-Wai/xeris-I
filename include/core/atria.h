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
 *  XERIS CORE
 *
 *  ATRIA
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  include/core/atria.h
 *  Definitions and Core Data Structures
 */

#ifndef __XERIS_CORE_ATRIA_H
#define __XERIS_CORE_ATRIA_H

#ifdef __XERIS_BUILD_ARCH_avr5
#include <arch/avr5/core/atria_avr5.h>
#else
#error "Target architecture not supported"
#endif

/* Operator/Desks */
#define OP_ATRIA_SECREG      0x0000  /* secreg$ */
#define OP_ATRIA_CTXM        0x0001  /* ctxm$   */
#define OP_ATRIA_INTR        0x0002  /* intr$ */
/* INTR$ Service codes */
#define SVC_ATRIA_INTR_REFISRT 0x0000

/* SECREG$ Service Codes */
#define ATRIA_SECREG_OPCERT  0x0000    /* [SRQ]OPCERT */

/* CTXM$ Service Codes */
#define ATRIA_CTXM_GEN       0x0000  /* General Initialization */


/* CTXM$ GEN Nominal Loader */
#define ATRIA_CTXM_GEN_NOMINAL  0x0000 /* Nominal Loader */


/* SISR Flags */
#define ATRIA_ISR_NEST  0x80      /* Nesting Enabled */
#define ATRIA_ISR_KIL6  0x40      /* Kill group 6    */
#define ATRIA_ISR_KIL5  0x20      /* Kill group 5    */
#define ATRIA_ISR_KIL4  0x10      /* Kill group 4    */
#define ATRIA_ISR_KIL3  0x08      /* Kill group 3    */
#define ATRIA_ISR_KIL2  0x04      /* Kill group 2    */
#define ATRIA_ISR_KIL1  0x02      /* Kill group 1    */
#define ATRIA_ISR_KIL0  0x01      /* Kill group 0    */



#endif
