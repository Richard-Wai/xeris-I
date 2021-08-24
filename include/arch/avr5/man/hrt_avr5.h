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

/*** for AVR_5 only! This file should be include from <xeris.h> ONLY! ***/

#ifndef __XERIS_MAN_HRT_H
#error "File shall be included from include/man/hrt.h ONLY."
#endif

#ifndef __XERIS_ATRIA_H_ARCH_avr5_H
#define __XERIS_ATRIA_H_ARCH_avr5_H 0.1

/* check for compatability with __flash and __memx */
#if !defined(__MEMX) || !defined(__FLASH)
#warning "AVR named spaces not supported by this version of gcc."
#warning "Build may fail. Please upgrade GCC to 6.1 or later."
#endif

/*  Heirarchical system allows for each image to include its own SIDXn
    as an extention of the root. ATRIA will initialize the root node
    entry for the given section identifier, and then link it to the
    packaged root SIDX node via a branch pointer */

/* Secretariat Index Node (SIDXn) */

struct hrt_sidxn
{  
    const __memx struct atria_hrtn *prev;   /* Previous node            */
    const __memx struct atria_hrtn *next;   /* Next node                */
    const __memx struct atria_hrtn *branch; /* Branch node              */
    
    const  __memx char             *nname;  /* Node human name string   */
    uint8_t                         ntype;  /* Node type                */

    uint16_t                        eid;    /* Element identifier       */
};

#endif

