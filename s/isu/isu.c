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
 *  s/isu%
 *  Support Group
 *  Case File Issue
 *
 *  isu.c
 *  Dispatch Desk
 */

#include <xeris.h>

/* Implemented in assembler */
extern void smcpy ( void * src, void * dst, uint8_t size );

uint16_t main ( xdcf * cf )
{
     /* Quite simply we verify caseboard is configured */
     /* properly, then copy the Case file pointeed to  */
     /* by the Case Board to the Active Case File      */

     if ( ( !cf->b.p ) || ( cf->b.s != sizeof ( xdcf ) ) )
     {
          /* According to specification, we need to clear */
          /* case board if it wasn't acceptable */
          cf->b.p = NULL;
          cf->b.s = 0;
          return ( 0 );
     }
     
     /* Good to go */
     smcpy ( cf->b.p, cf, sizeof ( xdcf ) );

     return ( 0 );
}

