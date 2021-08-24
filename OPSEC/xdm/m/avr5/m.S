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
; XDM$
; Executive Dispatch Marshal
;
; Machine-dependent Proceedures
;

;
; AVR 16-bit PC Implementation (No EIND)
; Compatible with all XERIS/APEX supported AVR
; models.
;


#include <sys/m.h>
#include <core/std/dispatch.h>


.globl opcon

.globl opsec_xdm_zcheck
.globl opsec_xdm_exec

        
.text

;
; extern void opsec_xdm_zcheck ( volatile struct dispatch_zone * )
;

;
; Zone Check:
; Check the current stack pointer against the zone shock,
; stack threshold, and stack limit
;

;
; zone in on r25:r24
;
; shock  * = +2
; thresh * = +4
; slim   * = +6
; stat   * = +10
;

opsec_xdm_zcheck:
    ; We will use Y pointer for this purpose,
    ; since it supports displacement
    push r16
    push r17
    push r18
    push r19
    push r28
    push r29

    ; Start with 100%
    ldi r17, DZ_ZONE_RDY
    
    ; load Y
    movw r28, r24

    ; make section critial
    in r16, r_SREG
    cli
    
    ; Load stack pointer to the original argument
    in r24, r_SPL
    in r25, r_SPH


    ; Just a bunch of fast checks
    ; load in threshold first
    ldd r18, Y+4
    ldd r19, Y+5
    
    ; check if SP is below (greater than) threshold
    sub r18, r24
    sbc r19, r25
    brmi 1f
    ; We are in warning territory
    ldi r17, DZ_ZONE_WRN

    ; ok, we are in warning territory now, but how bad is it?
    ; load stack limit
    ldd r18, Y+6
    ldd r19, Y+7

    ; is SP below (greater than) the limit?
    sub r18, r24
    sub r19, r25
    brmi 1f
    ; we have an Excursion, but is it containable?
    ldi r17, DZ_ZONE_LEX

    ; load zone shock
    ldd r18, Y+2
    ldd r19, Y+3

    ; is SP below the zone shock?
    sub r18, r24
    sub r19, r25
    brmi 1f
    ; uh oh..
    ldi r17, DZ_ZONE_ZEX

1:
    ; write-out new status code
    std Y+10, r17

    ; Bring back interrupts, if they were enabled
    out r_SREG, r16

    ; clean-up
    pop r29
    pop r28
    pop r19
    pop r18
    pop r17
    pop r16

    ret
    

;
; extern void opsec_xdm_exec
; (
;      volatile struct dispatch_zone             * outmar,
;      volatile struct dispatch_zone             * inmar,
;      volatile dispatch              * volatile * actd,
;      volatile dispatch                         * chkin
; )
;

;
; outmar  * = r25:r24
; inmar   * = r23:r22
; actd   ** = r21:r20
; chkin   * = r19:r18
;

opsec_xdm_exec:
    ; Things get weird. Since we are changing the stack in
    ; the middle, we are actually going through two "halfs"
    ; of this proceedure, the first half is out-marshal (now active)
    ; and the second half if in-marshal (next active)
    ; We need to block interrupts during this critial phase
    ; But need to be careful to not spend too much time

    ; First, a note on saved registers:
    ; We know XERIS/APEX Core is built with GCC, but we cannot assume
    ; all Secretariats are. Therefore full register saves need to
    ; be done at the actual pre-emption point, which can only be
    ; from an scheduler clock interrupt (as per the basic design
    ; of XERIS/APEX. XFM$ itself can only come in from a "clean"
    ; configuration, meaning we dont actually need to "save" all
    ; registers, only the ones we actully use.. Which incidentally
    ; makes non-preemptive Truck marshalling very quick. And
    ; pre-emption is only by scheduler clock, making full "context
    ; switches" very predictable, and limited to a maximum.
    
    ; Capture X, Y, and Z pointers
    push r31
    push r30
    push r29
    push r28
    push r27
    push r26


    ; SREG temp
    push r0
    
    ; We first need to load our pointers appropriately
    ; We want to use dispacement addressing since these
    ; are structures. So we use Y and Z registers
    ; X will be used as a general-purpose register
    ; Y will point to active ZONE file (out-marshal)
    ; z will point to the check-in ZONE file (in-marshal)

    ; Set-up Y (outmar)
    movw r28, r24

    ; Set-up Z (inmar)
    movw r30, r22

    ; It is time to lock down interrupts before we mark the SP
    ; We save SREG only for the Interrupt Flag, since we are not
    ; an isr, no one is waiting on that one from us
    ; But we can use it to pass through the system's interrupt config
    ; So we will actually pass SREG right into the next zone (in-marshal)
    in r0, r_SREG
    cli

    ; we need to take a moment to clear the preemption signal.
    ; This just ensures that a preemption will never preempt 
    ; another preeption until we're out of the marshal.
    ; if there's another preemption right after this,
    ; so be it.. but the tick rate is probably way too fast..

    ;load X pointer
    ldi r26, lo8(opint)
    ldi r27, hi8(opint)
    ;manually set the position of
    ;opint.sigpre 
    adiw r26, 0x06
    clr r1
    st X, r1
    
    ; Time is now of the essence
    ; Load in current Stack Pointer
    in r26, r_SPL
    in r27, r_SPH

    ; Now we just store the current Stack Pointer at
    ; the out-martial zone stack "mark"
    std Y+8, r26
    std Y+9, r27

    ; Now we are done with the out-marshal zone file
    ; so we will use the Y pointer to update the active dispatch
    ; pointer to the check-in dispatch file (check-in operation)
    movw r28, r20

    ; Simply write-out the new dispatch pointer!
    st Y+, r18
    st Y, r19


    ; Finally, we load the in-marshal zone stack mark
    ; and write it out to the SP
    ldd r26, Z+8
    ldd r27, Z+9

    ; Set new stack pointer and return
    ; We will set SPH, then reload SREG and then SPL
    ; This is because if interrupts are enabled, we are
    ; still garunteed that the next instruction is executed
    ; By doing it this way, we can ensure that any queued
    ; interrupt is handled as quickly as possible
    ; (immediately after we finished setting the new Stack Pointer
    out r_SPH, r27
    out r_SREG, r0
    out r_SPL, r26

    ; so here we are, in the new zone, now we clean up
    ; we will pop all the registers, which will be convenintly
    ; in place on the stack from the previous mashal out of this
    ; Truck..
    
    ; SREG temp
    pop r0

    ; X,Y,Z pointers
    pop r26
    pop r27
    pop r28
    pop r29
    pop r30
    pop r31

    ret
    
