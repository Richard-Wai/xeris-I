/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  XERIS CORE
 *
 *  FLATLINE%ULO$
 *  Umbilical Link Operator
 *
 */

#include <xeris.h>
#include <sys/types.h>
#include <core/flatline.h>

#include <flatline/flatline.h>


/* Service Agents */
static uint16_t ulo_svca_txstr   ( xdcf * cf );
static uint16_t ulo_svca_svcs    ( xdcf * cf );
static uint16_t ulo_svca_rxstr   ( xdcf * cf );

/* FLATLINE%ULO$ */
uint16_t flatline_ulo ( xdcf * cf )
{
     /* We will split up ULO$ into a few Service Agents */
     /* But first we need to determine where to direct  */
     /* The dispatch too.. */

     switch ( cf->w.s )
     {
          case XST_STR:
               return ( ulo_svca_txstr ( cf ) );
               break;

          case XST_SRQ:
               return ( ulo_svca_svcs ( cf ) );
               break;

          default:
                break;
     }

     /* If we get here, it means the manifest was not accepted */
     cf->w.s          = XST_DNY;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_F_UNACPT_ORDER;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;

     return ( 0 );
}


/* STR Manifest Service Agent (TX string) */
static uint16_t ulo_svca_txstr ( xdcf * cf )
{
     /* ticket is string type. Max is 8 char.  */
     /* We do not "require" a null-terminated  */
     /* String, but we will exit at null (not  */
     /* transmitted obviously). Otherwise, we  */
     /* will transmit 8. Packet sequence value */
     /* is ignored.                            */
     
     register uint8_t i;
     xdcf tcf;
     
     /* First we need to find out how many     */
     /* Character's we're dealing with, then   */
     /* We can just dispatch to ULIF$ in one-  */
     /* go. We will also copy the string over  */
     /* as we go. Neat. */

     /* catch an empty string */
     /* we turn it into a space so that we at- */
     /* least try to indicate that it happened */
     if ( cf->w.t.pstr.pld[0] == '\0' )
     {
          cf->w.t.pstr.pld[0] = ' ';
          cf->w.t.pstr.pld[1] = '\0';
     }
     
     for ( i = 0; i < 7; i++ )
     {
          tcf.w.t.srq.prm.p08[i] = (uint8_t)cf->w.t.pstr.pld[i];
          
          if ( cf->w.t.pstr.pld[i] == '\0' )
               break;
     }
     
     /* Set-up a dispatch for ULIF$ */
     tcf.w.o         = OP_FLATLINE_ULIF;      /* FLATLINE%ULIF$  */
     tcf.w.s         = XST_SRQ;                /* Service Request */
     tcf.w.t.srq.rqc = ULIF_TXx | (uint16_t)i; /* TX how many?    */

     /* Easy as pie. Thanks ULIF! */
     flatline_ulif ( &tcf );                  /* Execute */

     /* Tell everyone what a great job we did */
     cf->w.s          = XST_CPL;       /* Complete */
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_CPL;
     cf->w.t.rpt.cond = RPT_NRM;        /* All good */
     cf->w.t.rpt.note = RPT_NON;        /* Nothin to note */

     return ( 0 );
}

/* Services Redirection Handler */
static uint16_t ulo_svca_svcs ( xdcf * cf )
{
     
     if ( cf->w.t.srq.rqc == ULO_RXS )        /* RX String */
          return ( ulo_svca_rxstr ( cf ) );

     /* If we got here, the service code is invalid */
     cf->w.s          = XST_UNA;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_REQUEST_UKN;
     cf->w.t.rpt.cond = RPT_FTL;
     cf->w.t.rpt.note = RPT_NON;
     return ( 0 );
}

/* Receive String Service Agent */
static uint16_t ulo_svca_rxstr ( xdcf * cf )
{
     /* This service links fairly directly with similar ULIF$ */
     /* services, but returns the string as a standard        */
     /* string packet */

     xst_pstr buf;
     uint8_t len;
     register uint8_t i;

     /* Find the desired number of characters to receive.  */
     /* Maximum is 7 (7 + nul) If prm.p08[0] is more than  */
     /* 7, we will assume 7.                               */
     len = cf->w.t.srq.prm.p08[0];
     if ( len > 7 )
          len = 7;

     /* Before we get serrious, we will be smart and ensure  */
     /* The returning string will be nul-terminated properly */
     /* By filling it in (zeroing it out) before-hand        */
     for ( i = 0; i < 8; i++ )
          buf.pld[i] = '\0';
     
     /* Now we should formulate the correct service request */
     /* to ULIF$ to obtain the string */
     cf->w.o = OP_FLATLINE_ULIF;    /* FLATLINE%ULIF$  */
     cf->w.s = XST_SRQ;              /* Service Request */
     
     /* Maximum one-shot capability of ULIF$ is 4 characters */
     /* If len > 4, we will do two phases */
     cf->w.t.srq.rqc = (len > 4) ? ULIF_RX4 : ( ULIF_RXx + len );
     
     /* Phase 1 execute */
     flatline_ulif ( cf );

     /* Phase 1 copy */
     for
     (
          i = 0;
          i < ( (len > 4) ? 4 : len );
          i++
     )
          buf.pld[i] = (unsigned char)cf->w.t.rpt.adi.adi08[i];

     if ( len > 4 )
     {
          /* Phase 2 */
          cf->w.o = OP_FLATLINE_ULIF;
          cf->w.s = XST_SRQ;
          cf->w.t.srq.rqc = ULIF_RXx + len;
          flatline ( cf );

          for ( i = 4; i < len; i++ )
               buf.pld[i] =
                    (unsigned char)cf->w.t.rpt.adi.adi08[i - 4];
     }

     /* Write-out the received string */
     cf->w.s = XST_STR;

     for ( i = 0; i < 8; i++ )
          cf->w.t.pstr.pld[i] = ( i < len) ? buf.pld[i] : '\0'; 

     return ( 0 );
}




