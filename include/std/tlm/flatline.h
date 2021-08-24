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
 *  (xeris?soses*0011)
 *  <std/tlm/flatline.h>
 */

#ifndef __XERIS_STD_TLM_FLATLINE_H
#define __XERIS_STD_TLM_FLATLINE_H

#define TLM_FLATLINE            0x0000
#define TLM_ULIF                0x0001

#define TLM_OMNI_NOM            0x0002
#define TLM_OMNI_ALT            0x0003
#define TLM_OMNI_CHKP           0x0004


#define TLM_OMNI_RMEM_0         0x0010
#define TLM_OMNI_RMEM_1         0x0011
#define TLM_OMNI_RMEM_2         0x0012
#define TLM_OMNI_RMEM_3         0x0013
#define TLM_OMNI_RMEM_4         0x0014
#define TLM_OMNI_RMEM_5         0x0015
#define TLM_OMNI_RMEM_6         0x0016
#define TLM_OMNI_RMEM_7         0x0017
#define TLM_OMNI_RMEM_8         0x0018
#define TLM_OMNI_RMEM_9         0x0019
#define TLM_OMNI_RMEM_A         0x001a
#define TLM_OMNI_RMEM_B         0x001b
#define TLM_OMNI_RMEM_C         0x001c
#define TLM_OMNI_RMEM_D         0x001d
#define TLM_OMNI_RMEM_E         0x001e
#define TLM_OMNI_RMEM_F         0x001f

#define TLM_OMNI_XMEM_0         0x0020
#define TLM_OMNI_XMEM_1         0x0021
#define TLM_OMNI_XMEM_2         0x0022
#define TLM_OMNI_XMEM_3         0x0023
#define TLM_OMNI_XMEM_4         0x0024
#define TLM_OMNI_XMEM_5         0x0025
#define TLM_OMNI_XMEM_6         0x0026
#define TLM_OMNI_XMEM_7         0x0027
#define TLM_OMNI_XMEM_8         0x0028
#define TLM_OMNI_XMEM_9         0x0029
#define TLM_OMNI_XMEM_A         0x002a
#define TLM_OMNI_XMEM_B         0x002b
#define TLM_OMNI_XMEM_C         0x002c
#define TLM_OMNI_XMEM_D         0x002d
#define TLM_OMNI_XMEM_E         0x002e
#define TLM_OMNI_XMEM_F         0x002f

/* XERIS/APEX Version Information */
#define TLM_XERIS_REL           0xa000
#define TLM_XERIS_V_MAJ         0xa001
#define TLM_XERIS_V_MIN         0xa002
#define TLM_XERIS_COPY          0xa003
#define TLM_XERIS_LEG1          0xa004


#endif
