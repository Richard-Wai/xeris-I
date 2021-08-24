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
 *  XERIS Support Group
 *
 *  f/man%
 *  Human Relations Secretariat
 *
 *  GENERIC Release 1
 *  Version 1.0
 *
 *  man.c
 *  Dispatch Desk
 *
 */

#include <xeris.h>

#include <man/man.h>
#include <man/std/man.h>


uint16_t main ( xdcf * cf )
{
     /* Simple router */
     if ( cf->w.d == OP_F_MAN_TELCOM )
          return ( man_telcom ( cf ) );

     if ( cf->w.d == OP_F_MAN_STU )
          return ( man_stu ( cf ) );

     if ( cf->w.d == OP_F_MAN_ITOA )
          return ( man_itoa ( cf ) );


     /* Not recognised */
     cf->w.s          = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
     cf->w.t.rpt.cond = RPT_CNC;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}
