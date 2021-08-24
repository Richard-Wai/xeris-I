#include <xeris.h>

extern void spi_init ( void );
extern void spi_send ( xdcf * cf );

uint16_t main ( xdcf * cf )
{
     if ( !cf->w.d )
     {
          spi_init ( );
          return ( 0 );
     }

     spi_send ( cf );
     return ( 0 );
}
     
