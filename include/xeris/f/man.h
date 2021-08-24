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
 *  f/man%
 *  Human Relations Secretariat
 *
 *  <xeris/f/man.h>
 *  XERIS Specification Header
 */

#ifndef __XERIS_F_MAN_H
#define __XERIS_F_MAN_H

/* Secretariat */
#define X_F_MAN            0x1234

/* Desks */
#define X_F_MAN_STU        0x0001       /* man%stu$ */
#define X_F_MAN_ITOA       0x0008       /* man%itoa$   */
#define X_F_MAN_TELCOM     0x8000       /* man%telcom$ */


/* Tags */
/* itoa$ */
/* High nibble - signed/unsigned */
#define X_F_MAN_ITOA_UNSGN   0x0000
#define X_F_MAN_ITOA_SGN     0x0100

/* Low nibble - base */
#define X_F_MAN_ITOA_HEX     0x0000
#define X_F_MAN_ITOA_DEC     0x0001
#define X_F_MAN_ITOA_OCT     0x0002
#define X_F_MAN_ITOA_BIN     0x0003

/* telcom$ */
#define X_F_MAN_TELCOM_CON   0x0000
#define X_F_MAN_TELCOM_SEC   0x0001

#endif
