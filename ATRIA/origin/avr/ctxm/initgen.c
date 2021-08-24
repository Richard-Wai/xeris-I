/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2017, Richard Wai
 *  All rights reserved.
 */

/*
 *  XERIS CORE
 *
 *  ATRIA%
 *  Autonomous Terminal Registrar and Integration Authority
 *
 *  ATRIA/ORIGIN (AVR)
 *
 *  CTXM$
 *  Context Manager 
 *
 *  General Initialization
 *  Secondary Operator
 *  initgen.c
 *
 */

#include <xeris.h>
#include <core/atria.h>
#include <core/opsec.h>
#include <sys/m.h>

#include <atria/ctxm.h>
#include <atria/secreg.h>


void atria_ctxm_initgen ( xdcf * cf )
{
     uint16_t selsec;
     void *mext, *cext;
     void * (*ld_block) ( const __memx void *, void *, uint16_t );
     void * (*ld_bsclr) ( void *, uint16_t );
     
     struct atria_srf selsrf;
     volatile struct opsec_opcon * opcon =
          ((volatile struct opsec_opcon *)cf->b.p);
     
     /* Basically, we just cycle-through all available */
     /* Secretariat Registration Files, loading the    */
     /* .data and .bss sections by the selected loader */

     /* todo: add loader selection logic here */     
     ld_block = &atria_ctxm_ld0_block;
     ld_bsclr = &atria_ctxm_ld0_bsclr;

     /* First describe actual available memory */

     /* we will not run OPSEC (0000%) through this  */
     /* but we need to account for it, so we will   */
     /* manually initialized mext with OPSEC's info */
     atria_secreg_pullsrf ( 0, &selsrf );
     mext = (void *)( selsrf.bss.lda + selsrf.bss.siz );
     cext = (void *)( selsrf.data.lda + selsrf.data.siz );

     if ( cext > mext )
          mext = cext;


     /* We have a fast function, atria_secreg_nextsid, which */
     /* allows us to provide a starting point, which will    */
     /* obviously be FLATLINE%, and it will use an optimized */
     /* search to return the next available SID.             */
     /* When there are no more remaining, it returns 0000%   */

     selsec = 1;

     do
     {
          atria_secreg_pullsrf ( selsec, &selsrf );
          
          /* Handle OP1 */
          if ( selsrf.data.siz > 0 )
               cext = ld_block
               ( selsrf.data.ofs, selsrf.data.lda, selsrf.data.siz );

          /* check and update extents pointer */
          if ( cext > mext )
               mext = cext;

          /* Handle OP2 */
          if ( selsrf.bss.siz > 0 )
               cext = ld_bsclr ( selsrf.bss.lda, selsrf.bss.siz );
          
          if ( cext > mext )
               mext = cext;

          /* Lastly, check if there is a registered */
          /* Transaction Account. If so, we need to */
          /* Ensure the accounting paramaters are   */
          /* cleared. */

          if ( selsrf.soc.account )
          {
               selsrf.soc.account->act = 0;
               selsrf.soc.account->com = 0;
          }

          /* Seek next one */
          selsec = atria_secreg_nextsid ( selsec );
          
     } while ( selsec != 0 );

     /* all done, now configure the dispatch zone */
     opcon->active->zone->base = (void *)(c_RAMEND);
     opcon->active->zone->shock = mext;

     /* set up return ticket */
     cf->w.s = XST_RPT;
     cf->w.t.rpt.spec = XSR_OP;
     cf->w.t.rpt.code = XSR_OP_S_OPR_OK;
     cf->w.t.rpt.cond = RPT_NRM;
     cf->w.t.rpt.note = RPT_NON;

     return;
}

     
          




          


     
