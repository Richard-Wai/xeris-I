;
; XERIS/APEX System I
; Autonomous Poly-Executive
;
; Release 1
;
; Copyright (C) 2017, Richard Wai
; All rights reserved.
;

;
; XERIS CORE
;
; OPSEC%
; Operational Secretariat
;
; <opsec/sys.h>
; Low-Level System Proceedures
;
; Machine-dependent Proceedures
;

;
; AVR 16-bit PC Implementation (No EIND)
; Compatible with all XERIS/APEX supported AVR
; models.
;

#include <sys/m.h>

.globl sys_terminate
.globl sys_memcpy
.globl sys_flatline
.globl sys_atria

.text


;
; void sys_terminate ( void * ret, void * mark )
; Terminate Dispatch
;
; Set the stack pointer to the terminal mark
; Jump to the terminal return point
;
; void * ret  = r25:r24
; void * mark = r23:r22 


sys_terminate:

    ; If we are here, it probably means the stack is totally
    ; messed. So we can't use it anymore. This is a critical
    ; moment, so we will go into critical section immediately

    ; store SREG, go critical
    in r0, r_SREG
    cli
        
    ; EI jump uses the Z pointer, so load the pointer
    ; to the pointer first into Y
    movw r28, r24
    ld r30, Y+
    ld r31, Y

    ; set-up stack mark pointer pointer in Y pointer
    movw r28, r22

    ; reset stack pointer
    ld r26, Y+
    ld r27, Y
    out r_SPH, r27
    out r_SPL, r26

    out r_SREG, r0
    ijmp

;
; sys_memcpy ( void * src, void * dst, uint8_t size )
;
; Simple fast, light mem copy
;
; src:  r25:r24
; dst:  r23:r22
; size  r20
;

sys_memcpy:
    push r28
    push r29
    push r30
    push r31

    ; src in Y, dst in Z
    movw r28, r24
    movw r30, r22

    ; counter in 24
    clr r24
1:
    inc r24
    ld r22, Y+
    st Z+, r22

    cpse r24, r20
    rjmp 1b

    pop r31
    pop r30
    pop r29
    pop r28
    ret
    
;
; sys_flatline ( xdcf * )
;
; "Trampoline" to FLATLINE% Dispatch Desk
; Configured manually by ATRIA% itself
;

sys_flatline:
    jmp 0x0000
    
;
; sys_atria ( xdcf * )
;
; "Trampoline" to ATRIA% Dispatch Desk
; Configured manually by ATRIA% itself
;

sys_atria:
    jmp 0x0000


