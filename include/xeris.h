/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  xeris.h
 *
 *  Core Standards General Inclusion Group
 */

#ifndef __XERIS_H
#define __XERIS_H

#include <sys/types.h>
#include <std/xsts.h>
#include <std/xsdp.h>
#include <std/soses.h>

/* OPSEC CDD / "XERIS" Call */
#if defined(__XERIS_BUILD_ARCH_FAM_AVR)

#if defined(__XERIS_BUILD_ARCH_avr5)
#include <arch/avr5/xeris_avr5.h>


#else
#error "AVR - Architecture not supported."
#endif

#else
#error "Architecture family not supported."
#endif

/* BGSIS Standard Definitions */

/* Secretariats */

/* Operations Group */
#define XSEC_OPSEC     0x0000
#define XSEC_FLATLINE  0x0001
#define XSEC_ATRIA     0x0002

#define X_STD_ISR      0xf000



#endif
