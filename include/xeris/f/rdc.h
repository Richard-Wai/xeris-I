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
 *  XERIS Facilities Group
 *
 *  f/rdc%
 *  Recurrent Dispatch Commission
 *
 *  <xeris/f/rdc.h>
 *  XERIS Specification Header
 */

#ifndef __XERIS_F_RDC_H
#define __XERIS_F_RDC_H

/* Secretariat */
#define X_F_RDC            0x1005

/* Desks */
#define X_F_RDC_SUB        0x0000   /* f/rdc%sub$ */
#define X_F_RDC_CTL        0x0001   /* f/rdc%ctl$ */
#define X_F_RDC_AGT        0x0002   /* f/rdc%agt$ */
#define X_F_RDC_TXP        0x0003   /* f/rdc%txp$ */
#define X_F_RDC_TXE        0x0004   /* f/rdc%txe$ */


/* ctl$ Service Codes */
#define X_F_RDC_CTL_INIT   0x0000   /* ctl$init&  */
#define X_F_RDC_CTL_AGENT  0x0001   /* ctl$agent& */
#define X_F_RDC_CTL_CTRL   0x0002   /* ctl$ctrl&  */


#endif
