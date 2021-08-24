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
 *  s/txc%
 *  Support Group
 *  Ticket Exchange Secretariat
 *
 *  txc.c
 *  Dispatch Desk
 */

#include <xeris.h>
#include <xeris/s/txc.h>

/* Machine-optimized functions */
extern void txc_mcpy ( xst_std * src, xst_std * dst );


uint16_t main ( xdcf * cf )
{
     xst_std savetic;

     /* Verify case board is configured properly */

     if ( ( !cf->b.p ) || ( cf->b.s != sizeof ( xst_nul ) ) )
     {
          /* According to specification, we need to clear */
          /* case board if it wasn't acceptable */
          cf->b.p = NULL;
          cf->b.s = 0;
          return ( 0 );
     }

     /* Route based on incoming desk */
     switch ( cf->w.d )
     {
          case X_S_TXC_SWAP:
               txc_mcpy ( &cf->w.t, &savetic );
               txc_mcpy ( (xst_std *)cf->b.p, &cf->w.t );
               txc_mcpy ( &savetic, (xst_std *)cf->b.p );
               return ( 0 );
               break;

          case X_S_TXC_COPY:
               txc_mcpy ( &cf->w.t, (xst_std *)cf->b.p );
               return ( 0 );
               break;

          case X_S_TXC_PUT:
               txc_mcpy ( (xst_std *)cf->b.p, &cf->w.t );
               return ( 0 );
               break;

          default:
               /* Invalid desk */
               cf->b.p = NULL;
               cf->b.s = 0;
               break;
     }

     return ( 0 );
}
               
