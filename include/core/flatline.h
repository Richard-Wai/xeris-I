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
 *  FLATLINE%
 *  First Load Assesment Telemetry, Link Initialization for
 *  Nominal Entry
 *
 *  include/core/flatline.h
 *  Definitions, Core Data Structures and Prototypes
 */

#ifndef __XERIS_CORE_FLATLINE_H
#define __XERIS_CORE_FLATLINE_H


#include <xeris.h>
#include <core/atria.h>
#include <core/opsec.h>

/* Secretariat ID */
#define SEC_FLATLINE        0x0001     /* xeris?bgsis */

/* Operator IDs */

#define OP_FLATLINE_SYSBOOT 0x0001     /* SYSBOOT$ */
#define OP_FLATLINE_ULIF    0x0002     /* ULIF$    */

/* ULIF$ Service Request Codes */
#define ULIF_TXS 0x0000       /* Transmit String  */
#define ULIF_RXS 0X0001       /* Receive String   */
#define ULIF_INI 0x0002       /* Init Drivers     */
#define ULIF_RLS 0x0003       /* Release Drivers  */




#endif
