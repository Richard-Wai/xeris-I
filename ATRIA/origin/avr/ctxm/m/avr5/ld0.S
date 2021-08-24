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
;  ATRIA/ORIGIN (AVR)
;
;  CTXM$
;  Context Manager 
;
;  ld0
;  "Nominal Loader"
;

;
;  Assembly proceedures for higher speed
;

#include <sys/m.h>        
        
.globl atria_ctxm_ld0_block
.globl atria_ctxm_ld0_bsclr

.text

;
; void * atria_ctxm_ld0_block
; ( const __memx void * source, void * target, uint16_t run )
;
; Load a .data section block from source to target
;
; **Zero run size is checked from main program.**
; this procedure assumes non-zero run-size!
;
; source:  r24 ("RAMP") r23(MSB):r22
; target:  r21:r20
; run:     r19:r18
;
; ret:     r25:r24
;

atria_ctxm_ld0_block:
    push r1
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31
    clr r1

    ; Y pointer is target
    movw r28, r20

    ; first, we need to find where the source
    ; actually is
    mov r26, r24
    andi r26, 0x80
    brne ld_ram

ld_flash:
    movw r26, r18               ; get counter ready
    ; Z pointer is source

    ; RAMP?
    cpse r1, r24
    rcall ld_flash_rampset

    movw r30, r22

    ; here we go, simple loop
1:
    sbiw r26, 0x01  ; counter
    
    lpm r22, Z+
    st Y+, r22

    cpse r26, r1
    rjmp 1b
    cpse r27, r1
    rjmp 1b

    rjmp ld_block_closeup

ld_flash_rampset:
    ; do nothing for now
    ret


ld_ram:
    ; Z pointer is source in ram
    
    ; RAMP?
    andi r24, 0x7F  ; eliminate high bit
    cpse r24, r1
    rcall ld_ram_rampset

    movw r30, r22
    movw r26, r18

    ; here we go, simple loop
1:
    sbiw r26, 0x01  ; counter
    
    ld r22, Z+
    st Y+, r22

    cpse r26, r1
    rjmp 1b
    cpse r27, r1
    rjmp 1b

    rjmp ld_block_closeup



ld_ram_rampset:
    ; do nothing for now
    ret

ld_block_closeup:
    ; return pointer to one past end
    movw r24, r28
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r1
    ret

;
; void * atria_ctxm_ld0_bsclr
; ( void * target, uint16_t run )
;
; Clear a .bss section block
;
; **Zero run size is checked from main program.**
; this procedure assumes non-zero run-size!
;
; target:  r25:r24
; run:     r23:r22
;
; ret:     r25:r24
;

atria_ctxm_ld0_bsclr:
    push r1
    push r28
    push r29
    push r30
    push r31
    clr r1

    movw r30, r22
    ; Y pointer is target
    movw r28, r24

    ; here we go, simple loop
1:
    sbiw r30, 0x01  ; counter
    
    st Y+, r1

    cpse r30, r1
    rjmp 1b
    cpse r31, r1
    rjmp 1b


    ; return the pointer
    movw r24, r28
    pop r31
    pop r30
    pop r29
    pop r28
    pop r1
    ret  

