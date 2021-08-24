/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All rights reserved.
 */

/*
 *  XERIS Facilities Group
 *
 *  f/man%
 *  Human Relations Secretariat
 *
 *  <man/man.h>
 *  Global definitions
 *
 */

#ifndef __F_MAN_H
#define __F_MAN_H

#include <xeris.h>

#define BUFSTD  83       /* Standard buffer size */

/* Dispatch Desk */
extern uint16_t main ( xdcf * );

/* Service Desks */
extern uint16_t man_stu    ( xdcf * );   /* 0001$ STU$    */
extern uint16_t man_itoa   ( xdcf * );   /* 0008$ ITOA$   */
extern uint16_t man_telcom ( xdcf * );   /* 8000$ TELCOM$ */

/* Internal Assistants */
extern void     ld_man   ( uint16_t idx,
                           unsigned char * dst, uint16_t len );
extern void     ld_soses ( uint16_t spec, uint16_t code,
                           unsigned char * dst, uint16_t len );
extern void     memcp    ( void * src, void * dst, uint16_t len );

extern void     insrt    ( unsigned char * buf, unsigned char * ins );

extern void     itostr   ( unsigned char * buf, uint32_t, uint8_t base );
extern uint32_t strtoi   ( unsigned char * buf, uint8_t base );

extern void rpt_reject_ptr ( xdcf * cf );


#endif
