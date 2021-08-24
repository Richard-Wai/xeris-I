/* User Process 0    */


#include <xeris.h>
#include <core/flatline.h>

#include <xeris/s/txc.h>
#include <xeris/f/rdc.h>

extern void clight ( void );
static uint16_t init ( xdcf * cf );
static uint16_t sc   ( xdcf * cf );

uint16_t main ( xdcf * cf )
{
     if ( cf->w.s == XST_SRQ )
          return init ( cf );
     else
          return sc ( cf );
}

static uint16_t init ( xdcf * cf )
{
     /* put out our own event tickets */
     cf->w.d = X_F_RDC_SUB;
     cf->w.s = XST_EVT;
     cf->w.t.evt.dsk = 0;
     cf->w.t.evt.ref = 1;       /* Cycle led */
     cf->w.t.evt.seq = 0;
     xeris ( X_F_RDC );

     cf->w.d = X_F_RDC_SUB;
     cf->w.s = XST_EVT;
     cf->w.t.evt.dsk = 0;
     cf->w.t.evt.ref = 2;       /* Print 1 */
     cf->w.t.evt.seq = 0;
     xeris ( X_F_RDC );

     cf->w.d = X_F_RDC_SUB;
     cf->w.s = XST_EVT;
     cf->w.t.evt.dsk = 0;
     cf->w.t.evt.ref = 3;       /* Print 2 */
     cf->w.t.evt.seq = 0;
     xeris ( X_F_RDC );

     return ( 0 );

     cf->w.s = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_I_INIT_OK;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

}

static uint16_t sc ( xdcf * cf )
{
     xst_evt savevt;
     unsigned char out[2];


     if ( cf->w.t.evt.ref == 1 )
     {
          clight ( );
          return ( 0 );
     }

     out[1] = '\0';

     if ( cf->w.t.evt.ref == 2 )
          out[0] = '1';
     else if ( cf->w.t.evt.ref == 3 )
          out[0] = '2';
     else
          out[0] = '*';


     savevt = cf->w.t.evt;
     
     cf->b.p = (void *)out;
     cf->b.s = 2;
     cf->w.d = 0x0002;           /* ULIF$ */
     cf->w.s = XST_SRQ;
     cf->w.t.srq.rqc = 0;       /* TX string */
     xeris ( 1 );               /* flatline% */

     cf->b.p = NULL;
     cf->b.s = 0;
     cf->w.s = XST_EVT;
     cf->w.t.evt = savevt;

     return ( 0 );
}
          
          
          

     






     

          
               
          

     

