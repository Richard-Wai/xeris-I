/*
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  XERIS CORE
 *
 *  FLATLINE%ULIF$
 *  Umbilical Link Interface Operator
 *
 */

#include <xeris.h>
#include <sys/types.h>
#include <core/flatline.h>

#include "if.h"

/* Local helper functions ("Service Agents") */
static uint16_t ulif_svca_ifinit ( xdcf * cf );  /* ULIF_INI# */
static uint16_t ulif_svca_ifrels ( xdcf * cf );  /* ULIF_RLS# */
static uint16_t ulif_svca_txstr  ( xdcf * cf );  /* ULIF_TXx# */
static uint16_t ulif_svca_rxstr  ( xdcf * cf );  /* ULIF_RXx# */

/* FLATLINE%ULIF$ */
uint16_t flatline_ulif ( xdcf * cf )
{
     
     /* As per specification, ULIF$ does not accept */
     /* tickets which are not Service Requests */
     if ( cf->w.s != XST_SRQ )
     {
          cf->w.d          = OP_FLATLINE_ULIF;
          cf->w.s          = XST_RJT;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
          cf->w.t.rpt.cond = RPT_FTL;
          cf->w.t.rpt.note = RPT_NON;
          
          return ( 0 );
     }

     /* Here are all the service handlers (agents) */

     /* ULIF_TXS# - Transmit String */
     if ( cf->w.t.srq.rqc == ULIF_TXS )
          return ( ulif_svca_txstr ( cf ) );

     /* ULIF_RXS# - Receive Characters */
     if ( cf->w.t.srq.rqc == ULIF_RXS )
          return ( ulif_svca_rxstr ( cf ) );
     
     /* ULIF_INI# - Init Request */
     if ( cf->w.t.srq.rqc == ULIF_INI )
          return ( ulif_svca_ifinit ( cf ) );

     /* ULIF_RLS# - Release Request */
     if ( cf->w.t.srq.rqc == ULIF_RLS )
          return ( ulif_svca_ifrels ( cf ) );

     /* Default - RQC not valid / recognised */
     cf->w.s = XST_UNA;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_UKN;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );

}


/* Service Agents */

/* ULIF_INI# Service Agent */
static uint16_t ulif_svca_ifinit ( xdcf * cf )
{
     cf->w.t.rpt.adi.adi32 = 0;        /* Make sure ADI is zero-ed out */

     /* Execute */
     cf->w.t.rpt.adi.adi16[0] = ulif_if_init ( );
     
     if ( (cf->w.t.rpt.adi.adi16[0] & 0x00ff) == 0 )  /* No TX Lines!! */
     {
          cf->w.s           = XST_UNA;
          cf->w.t.rpt.spec  = XSR_OP;
          cf->w.t.rpt.code  = XSR_OP_F_INIT_FATAL;
          cf->w.t.rpt.cond  = RPT_FTL;
          cf->w.t.rpt.note  = RPT_STA;

          return ( 0 );
     }

     if ( (cf->w.t.rpt.adi.adi16[0] & 0xff00) == 0 )    /* Only TX */
     {
          cf->w.s          = XST_PAR;
          cf->w.t.rpt.spec = XSR_OP;
          cf->w.t.rpt.code = XSR_OP_S_WRITE_ONLY;
          cf->w.t.rpt.cond = RPT_OKY;
          cf->w.t.rpt.note = RPT_STA;
     
          return ( 0 );
     }

     /* We have a bi-directional line! */
     cf->w.s          = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_I_INIT_OK;
     cf->w.t.rpt.note = RPT_STA;
     
     return ( 0 );
}


static uint16_t ulif_svca_ifrels ( xdcf * cf )
{
     /* Not much to really do here... */
     ulif_if_rels ( );

     cf->w.s          = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}


static uint16_t ulif_svca_txstr ( xdcf * cf )
{
     register uint16_t i;
     register unsigned char c;

     for ( i = 0; i < cf->b.s; i++ )
     {
          c = ((unsigned char *)cf->b.p)[i];
          
          if ( c )
               ulif_if_txchar ( c );
          else
               break;
     }

     cf->w.s          = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}

static uint16_t ulif_svca_rxstr ( xdcf * cf )
{
     register uint8_t i;

     /* Clear out return data */
     cf->w.t.rpt.adi.adi32 = 0;
     
     for ( i = 0; i <= cf->b.s; i++ )
          ((unsigned char *)cf->b.p)[i] =
               ulif_if_rxchar ( );

     cf->w.s          = XST_CPL;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_RET;

     return ( 0 );
}
