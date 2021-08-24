;
;  XERIS/APEX System I
;  Autonomous Poly-Executive
;
;  Release 1
;
;  Copyright (C) 2017, Richard Wai
;  All rights reserved.
;

;
;  XERIS CORE
;
;  ATRIA%
;  Autonomous Terminal Registrar and Integration Authority
;
;  Core Interface Module
;

.globl atria_secreg_alpha
.globl intr_sisr

.text

atria_secreg_alpha:   ; external symbol for SRF tree root
intr_sisr:            ; SISR table pointer
   .word 0x0000
    
