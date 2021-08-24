/* Timer tester, basic timer */

#include <xeris.h>

extern void clock_init ( void );

uint16_t main ( xdcf * cf )
{
     /* We only do inits */
     clock_init ( );
     return ( 0 );
}
     
