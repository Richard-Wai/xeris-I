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
; SIPLEX%
; Strategic Imparative Planning Liason Executive
;
; SIPLEX/STARLINE (AVR5)
;
; seed.S
;

#include <sys/m.h>
        
.text

.globl opcon

.globl executor
.globl bootstrap
.globl exec_boot
.globl exec_seed
.globl exec_crank
.globl exec_reset

;
; void exec_boot ( void * base ) 
;
; Set SP to indicated base and jump to bootstrap
;
; base *: r25:r24
;
exec_boot:
    in r0, r_SREG
    cli

    out r_SPH, r25
    out r_SREG, r0
    out r_SPL, r24

    jmp bootstrap

new_exec:
    ; When we come in on a newly seeded dispatch
    ; we are invariably entered from OPSEC%XDM$,
    ; which in turn is called from OPSEC%XS%.
    ; This means that XS$ is still "running",
    ; and is "waiting" for us to return so it
    ; can clear the opcon.xs_in semaphore..
    ; so we need to do this for XS, or everything
    ; grinds to a halt since XS$ will never run again
    ldi r28, lo8(opcon)
    ldi r29, hi8(opcon)
    clr r1
    ; local opcon pointer is in Y
    ; but we need to load it to Z
    ld r30, Y+
    ld r31, Y
    
    ; opcon now pointed to by Z
    ; xs_in is single byte,
    ; we need to clear it
    ; this needs to be atomic, which it is here
    std Z+6, r1  ; opcon->xs_in;

    ; good to go, continue as a normal "reset"
    jmp exec_reset
    
        
;
; void exec_seed ( volatile struct lane * tlane )
;
; *tlane: r25:r24
;

;
; Loads a simulated stack frame that mates with OSPEC%XDM$
; when being marshalled in, which will then call to exec_reset
; directly. This proceedure is intended for cold-starting
; new lanes
;

exec_seed:
    ; secure and load Y and Z pointers

    push r26
    push r27
    push r28
    push r29
    push r30
    push r31

    ; first we need to follow the lane configuration
    ; to find our zone file
    ; first load Y with pointer to the lane file
    ; our ultimate goal is to get Z is the zone
    ; mark _pointer_, and Y as being the mark (void *)itself
    movw r28, r24
    ldd r30, Y+6  ; lane.commission (dispatch *)
    ldd r31, Y+7
    ldd r28, Z+11 ; dispatch.zone
    ldd r29, Z+12
    ; Y now points to the top of the zone file
    ; the zone mark is offset +8 bytes
    ; Z should be is Y+8
    movw r30, r28
    adiw r30, 0x08
    ; Z is set

    ; now let's initialize the mark to the zone
    ; base
    ld r26, Y  ; zone.base
    ldd r27, Y+1
    movw r28, r26
    ; Y is set

    ; to re-cap
    ; Z: Pointer to zone.mark 
    ; Y: actual mark

    ; Y is now the bottom of the viritual/future stack
    ; so first we need to push the "return" pointer (new_exec)
    ; note that in the real stack, it is post decrement on push
    ; but we don't have post-decrement for store, so we need to
    ; use pre-increment with a +1 at the end, before we store the new
    ; mark

    ldi r26, lo8(new_exec)
    ldi r27, hi8(new_exec)
    ; but wait, there's more!
    ; avr gcc works on a flat 24-bit address space in the compiler
    ; the linker translates the flash addresses (<800000) to the
    ; AVR native word address. So when we load the address above
    ; it is a byte address! so we need to divide by two first
    lsr r27
    ror r26

    ; so first we need to bury a little nugget
    ; when the "return" goes "back" to exec_reset
    ; exec_reset will be expecting a pointer to the relevent
    ; lane pointer (we received as an argument) on the stack
    ; push high byte, then low
    st Y, r25
    st -Y, r24

    ; now push our fake "return" address
    ; on avr calls, the return *low* byte is (paradoically)
    ; pushed first. That means it appears on the stack in
    ; big-endian order, unlike everything else in memory
    st -Y, r26
    st -Y, r27

    ; "push" our imaginary in-marshal stack
    ; see OPSEC%XDM$ (opsec_xdm_exec)
    clr r26
    st -Y, r26      ;r0
    st -Y, r26      ;r26
    st -Y, r26      ;r27
    st -Y, r26      ;r28
    st -Y, r26      ;r29
    st -Y, r26      ;r30
    st -Y, r26      ;r31

    ; ok, now we back off the pointer one more,
    ; and write it back out to "mark"
    sbiw r28, 0x01
    st Z, r28
    std Z+1, r29

    ; lane is good to go!


    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    ret

;
; void exec_crank ( volatile struct lane * )
;
; This is for bootstrap (INIT$) to call on one lane
; this simply places the lane file pointer properly
; in the path of exec_reset, so that we dont need
; to come in from OPSEC%XDM$. This is used on one
; lane (lane 0) at the end of INIT$ to "crank" the
; system into action
;
; lane *: r25:r24
;


exec_crank:
    ; simply push it down
    ; high byte first
    push r25
    push r24

    ; and run right into exec!

        
;
; void exec_reset ( void )
;
; This proceedure is usually called when
; a executor has completed a shot cycle
; this will reset the zone stack
;

exec_reset:
    ; we are about to obliterate the stack
    ; so forget pushing anything, this is the real deal
    ; grab the lane pointer first, and load it into Y
    pop r28
    pop r29
    clr r1

    ; look up the zone file, and put the pointer in Z
    ; using X on the way
    ldd r26, Y+6  ; lane.commission (dispatch *)
    ldd r27, Y+7

    adiw r26, 0x0b ; dispatch.zone
    ld r30, X+
    ld r31, X

    ld  r26, Z    ; zone.base
    ldd r27, Z+1


    
    ; go critical
    in r0, r_SREG
    cli

    ; reset zone status, set SP
    std Z+10, r1  ; zone.stat
    out r_SPL, r26
    out r_SREG, r0              ; de-critical
    out r_SPH, r27

    ; set up arguments for void executor ( struct lane * )
    movw r24, r28

    ; git 'er done
    jmp executor
    

    
    

