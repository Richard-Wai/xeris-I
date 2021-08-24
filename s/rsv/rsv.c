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
 *  s/rsv%
 *  Support Group
 *  Case File Reserve
 *
 *  rsv.c
 *  Dispatch Desk
 */

#include <xeris.h>

/* Implemented in assembler */
extern void smcpy ( void * src, void * dst, uint8_t size );

uint16_t main ( xdcf * cf )
{
     struct xsdp_case_board_s saveboard;
     
     /* Quite simply we verify caseboard is configured */
     /* properly, then copy the Active Case File to the */
     /* Case Board */

     if ( ( !cf->b.p ) || ( cf->b.s != sizeof ( xdcf ) ) )
     {
          /* According to specification, we need to clear */
          /* case board if it wasn't acceptable */
          cf->b.p = NULL;
          cf->b.s = 0;
          return ( 0 );
     }
     
     /* We need to preserve the board of the destination */
     /* before we do the copy */
     saveboard.p = ((xdcf *)cf->b.p)->b.p;
     saveboard.s = ((xdcf *)cf->b.p)->b.s;
     
     smcpy ( cf, cf->b.p, sizeof ( xdcf ) );

     /* Restore */
     ((xdcf *)cf->b.p)->b.p = saveboard.p;
     ((xdcf *)cf->b.p)->b.s = saveboard.s;

     return ( 0 );
}

