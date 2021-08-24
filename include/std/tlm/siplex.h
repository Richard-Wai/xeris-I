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
 *  FLATLINE% Specific Telemetry Encoding
 *  (xeris?soses*0013)
 *  <std/tlm/flatline.h>
 */

#ifndef __XERIS_STD_TLM_SIPLEX_H
#define __XERIS_STD_TLM_SIPLEX_H

/* Configuration Telemetry */
#define TLM_SIPLEX_CFG_PKG_STARLINE  0x0000

#define TLM_SIPLEX_CFG_ARCH          0xb000
#define TLM_SIPLEX_CFG_DEV           0xb001

#define TLM_SIPLEX_CFG_VMAG          0xb002
#define TLM_SIPLEX_CFG_VMIN          0xb003

#define TLM_SIPLEX_CFG_LANES         0xb010
#define TLM_SIPLEX_CFG_QUEUES        0xb011
#define TLM_SIPLEX_CFG_ZONES         0xb020
#define TLM_SIPLEX_CFG_INTZ          0xb030

#endif
