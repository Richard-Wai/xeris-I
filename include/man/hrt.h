/*  
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 *
 */

/*
 *  include/man/hrt.h
 *  Standard definitions and data structures for
 *  the Human Referance and Translations Secretariat,
 *  of the Human Relations Group.
 *  xeris!man/hrt%
 *
 */

#ifndef __XERIS_MAN_HRT_H
#define __XERIS_MAN_HRT_H

#ifdef __XERIS_BUILD_ARCH_avr5
#include <arch/avr5/man/hrt_avr5.h>
#else
#error "Target architecture not supported"
#endif


/* Standard Definitions */

/* Node Types */
#define HRT_SIDX_SEC    0x00            /* Secretariat ID  (%) */
#define HRT_SIDX_OPR    0x10            /* Operator ID     ($) */
#define HRT_SIDX_OPS    0x20            /* Service Code    (&) */
#define HRT_SIDX_REF    0x30            /* Local reference (#) */
#define HRT_SIDX_LBR    0x40            /* Logical Branch  (/) */


#define HRT_SIDX_BRR    0xf0            /* Branch Return Reference   */
#define HRT_SIDX_SRE    0xf1            /* Strand Run-Time Extension */



#endif
