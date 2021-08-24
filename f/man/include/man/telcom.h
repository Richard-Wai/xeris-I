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
 *  XERIS Support Group
 *
 *  man%
 *  Human Relations Secretariat
 *
 *  telcom$
 *  Telemetry Commissioner
 *
 *  <man/tc.h>
 *  Standard definitions
 *
 */

#ifndef __F_MAN_TELCOM_H
#define __F_MAN_TELCOM_H

#include <xeris.h>

/* Configuration File */
struct tc_cfg_s
{
     uint16_t cfg_auth;      /* Config minimum downstream security level */

     uint16_t console_sid;   /* Console Driver SID  */
     uint16_t console_dsk;   /* Console Driver DESK */

};

/* Config block */
extern struct tc_cfg_s tc_cfg;

/* Specification Constants */
#define TELCOM_CFG_CON 0x0000     /* Console Config              */
#define TELCOM_CFG_SEC 0x0001     /* Minimum config access level */

/* Helpers */
extern void telcom_fetch_message
( unsigned char * buf, uint16_t spec, uint16_t code );

extern void telcom_fetch_format
( unsigned char * buf, uint16_t spec, uint16_t code );

#endif
