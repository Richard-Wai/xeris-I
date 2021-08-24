/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All Rights Reserved.
 */

/*
 *  XERIS Support Group
 *
 *  f/man%
 *  Human Relations Secretariat
 *
 *  GENERIC Release 1
 *  Version 1.0
 *
 *  <std/f/man.h>
 *  "std.h"
 *  Specification Header
 *
 */

/* Desk Definitions */
#define OP_F_MAN_STU      0x0001       /* man%stu$ */
#define OP_F_MAN_ITOA     0x0008       /* man%itoa$   */
#define OP_F_MAN_TELCOM   0x8000       /* man%telcom$ */


/* ITOA Service Request Codes */

/* High nibble - signed/unsigned */
#define SVC_MAN_ITOA_UNSGN   0x0000
#define SVC_MAN_ITOA_SGN     0x0100

/* Low nibble - base */
#define SVC_MAN_ITOA_HEX     0x0000
#define SVC_MAN_ITOA_DEC     0x0001
#define SVC_MAN_ITOA_OCT     0x0002
#define SVC_MAN_ITOA_BIN     0x0003



