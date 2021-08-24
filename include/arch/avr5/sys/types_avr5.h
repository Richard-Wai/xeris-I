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
 *  types_(arch).h (arch/avr5/xeris/types_avr5.h)
 *
 *  Standard type definitions for avr5 architecture
 */

#ifndef __XERIS_SYS_TYPES_H
#error "File shall be included from <sys/types.h> ONLY."
#endif

#ifndef __XERIS_SYS_TYPES_H_ARCH_avr5_H
#define __XERIS_SYS_TYPES_H_ARCH_avr5_H 1

#define NULL ( (void *) 0 )

// Variable types
typedef unsigned char       uint8_t;
typedef unsigned int        uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned long long  uint64_t;
typedef char                int8_t;
typedef int                 int16_t;
typedef long                int32_t;
typedef long long           int64_t;

typedef uint16_t            size_t;
typedef uint16_t            ptr_t;  // pointer size

/* define a variable able to hold a __memx pointer (24 bits) */
typedef uint32_t            _avr_memxp_t;

/* define control signals for this system */
typedef int16_t signal;

#endif
