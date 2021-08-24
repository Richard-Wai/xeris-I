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
 *  OPSEC%
 *  Operational Secretariat
 *
 *  XDM$
 *  Executive Dispatch Marshal
 *
 *  opsec/xdm/xdm.c
 *  Main Module
 */

#include <xeris.h>
#include <core/std/dispatch.h>
#include <opsec/opsec.h>
#include <opsec/xdm.h>

#include "m.h"


void opsec_xdm ( volatile dispatch * in_marshal )
{

     /* Check if in_marshal status is ok (ASN/WRN) */
     if ( in_marshal->zone->stat != DZ_ZONE_RDY )
          return;   /* Abort */

     /* Next bit is assembler to be as fast as possible            */
     opsec_xdm_exec
     (
          opcon.active->zone,
          in_marshal->zone,
          (dispatch **)(&opcon.active),
          in_marshal
     );

     return;

}



     
