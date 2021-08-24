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
 *  f/rdc%
 *  Recurrent Dispatch Commission
 *
 *  Dispatch Desk
 */

#include <xeris.h>
#include <xeris/f/rdc.h>

#include <f/rdc/rdc.h>


uint16_t main ( xdcf * cf )
{
     /* Route desks as usual */

     if ( cf->s.sid == X_F_RDC )
     {
          /* Internal desks */
          if ( cf->w.d == X_F_RDC_AGT )
                    /* agt$ - agent process */
                    return agt ( cf );
          
          if ( cf->w.d == X_F_RDC_TXP )
                    /* txp$ - push transaction */
                    return txp ( cf );

          if ( cf->w.d == X_F_RDC_TXE )
                    /* txe$ - execute transactions */
                    return txe ( );
     }
     else
     {          
          if ( cf->w.d == X_F_RDC_SUB ) 
               /* sub$ */
               return sub ( cf );

          if ( cf->w.d == X_F_RDC_CTL )
               /* ctl$ */
               return ctl ( cf );
     }

     
     /* Desk does not match */
     cf->w.s = XST_RJT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_NO_OPERATOR;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}


               
