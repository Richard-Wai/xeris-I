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
 *  XERIS CORE
 *
 *  CORE STANDARDS
 *
 *  <core/std/dispatch.h>
 *  Dispatch Package and Operational File Definitions
 *
 *  <arch/avr5/std/dispatch.h>
 *  AVR5 Target Architecture
 */


#ifndef __XERIS_CORE_STD_DISPATCH_H_ARCH_avr5_H
#define __XERIS_CORE_STD_DISPATCH_H_ARCH_avr5_H

#include <xeris.h>
#include <core/atria.h>

/* Dispatch Zone File */

/* dispatch_zone shall always be referenced/declared as volatile */

struct dispatch_zone
{
     void     * base;      /* Zone base                     */
     void     * shock;     /* Zone shock                    */
     
     void     * thresh;    /* Stack threshold               */
     void     * slim;      /* Stack limit                   */
     void     * smark;     /* Stack mark                    */

     uint8_t    stat;      /* Status Code                   */
     
};

/* Dispatch Supervisor File */
/* dispatch_super shall always be referenced/declared as volatile */

struct dispatch_super
{
     struct atria_soc  cosec; /* Check-out Secretariat */
     struct atria_soc  cisec; /* Check-in  Secretariat */

     xdcf              scase; /* Supervisor Case File  */

     volatile void   * termret;   /* Terminal Return     */
     volatile void   * termmrk;   /* Terminal Stack Mark */

     volatile void   * exec;      /* Executor Reference */
};

/* Dispatch Transaction File */
/* dispatch_transact shall always be referenced/declared as volatile */
struct dispatch_transact
{
     uint16_t         (*volatile exec) (xdcf *);  /* Transaction executive */
     void           * mark;                       /* Stack mark            */
     
     uint16_t         ctarget;                    /* Commit Target         */
     uint16_t         amark;                      /* Act Semaphore Mark    */
};

#endif
