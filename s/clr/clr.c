/*
 *  XERIS Support Group
 *
 *  s/clr%
 *  Support Group
 *  Case File Clear
 *
 *  clr.c
 *  Dispatch Desk
 *
 */

#include <xeris.h>

/* Implemented in assembler */
extern void mclr ( xdcf * tgt, uint8_t size );

uint16_t main ( xdcf * cf )
{
     /* Our job is to clear the case file *entirely*  */
     mclr ( cf, sizeof ( xdcf ) );
     return ( 0 );
}

