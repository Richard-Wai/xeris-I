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
; XS$
; Executive Scheduler
;
; Machine-dependent Proceedures
; AVR5
;

#include <sys/m.h>

.globl lock_xs
.globl free_xs
.globl clear_atomic16        

.text        

;
; uint8_t lock_xs ( volatile uint8_t * xs_in )
;
; Attempt to lock xsin semaphore. Return 0xff if successful
; Return 0x00 if not successful
;
; * xs_in : r25:r24
; ret:     r24

lock_xs:
    push r0
    ; clear out X pointer
    push r26
    push r27    

    ; set up X and return
    movw r26, r24
    clr r24
    
    ; go critical
    in r0, r_SREG
    cli

    ld r25, X
    ; is xsin zero?
    tst r25
    brne 1f
    
    ; xsin is zero
    ; write out exactly one
    ldi r25, 0x01
    st X, r25

    ; set status
    ser r24

1:
    ; done, clean-up
    out r_SREG, r0
    clr r25
    pop r27
    pop r26
    pop r0
    ret

;
; void free_xs ( volatile uint8_t * xs_in )
;
; Attempt to lock xsin semaphore. Return 0xff if successful
; Return 0x00 if not successful
;
; * xs_in : r25:r24
;   

free_xs:
; go critical and clear xsin

; aquire X pointer
push r0
push r26
push r27

; set X pointer
movw r26, r24

; go critical
in r0, r_SREG
cli

clr r1
out r_SREG, r0
st X, r1

pop r27
pop r26
pop r0
ret

; clear a 16-but value atomically

clear_atomic16:
    ; save sreg in r0 and clear interrupts
    push r0
    push r26  ; X pointer
    push r27

    movw r26, r24
    clr r24
    
    in r0, r_SREG
    cli

    st X+, r24
    out r_SREG, r0
    st X, r24

    pop r27
    pop r26
    pop r0
    ret

    



    
    

    
