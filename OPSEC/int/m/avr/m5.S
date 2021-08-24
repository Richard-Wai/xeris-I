;
; XERIS/APEX System I
; Autonomous Poly-Executive
;
; Release 1
;
; Copyright (C) 2016, Richard Wai
; ALL RIGHTS RESERVED
;

;
; XERIS CORE
;
; OPSEC%
; Operational Secretariat
;
; INT$
; Interrupt Response Coordinator
;
; Machine-dependent Proceedures
; AVR5
;

#include <sys/m.h>

; variables
; extern struct opsec_opcon_int opint
.globl opint
; extern const __memx struct atria_sisr * const __memx * opint_isr
.globl opint_isr


; Built-in library exclusion
.globl __do_copy_data

; Vector handling
.globl vectors
.globl opsec_int_enter

; system globals
.globl sys_boot
        
;
; Vector Map
;

#if defined(__XERIS_BUILD_DEV_atmega328p)
#include "vectors/avr5/mega328p.S"

#else
#error "INT$ for AVR5: Device not supported
#endif

.text

;
; void opsec_int_enter  ( void )
;
; Interrupt Coordinator main entry
; We are jumped here from the vector
; IRQ number is in r24
;
opsec_int_enter:
    ; interrupt number is in r24

    ; save SREG
    push r25
    in r25, r_SREG
    push r25

    push r0
    push r28
    push r29
    push r30
    push r31

    ; save the irq number
    mov r0, r24

    ; check if we're the first in
    ; (stack mark is null), if so we switch to the
    ; interrupt stack. If we arn't first, we make
    ; sure the interrupt stack is below the treshold
    ; if it trips, we will enable throttling.
    ; throttleling needs to be turned off by someone
    ; else. Both paths will use the same number of
    ; cycles, making interrupt latency 100%
    ; deterministic

    ; load in opint to Y pointer directly
    ldi r28, lo8(opint)
    ldi r29, hi8(opint)

    ; get the "mark"
    ; the mark is used to store the stack we need to
    ; return to after all of the interrupts close-out
    ldd r24, Y+4 ; smark
    ldd r25, Y+5
    ; check if we are "first in" (mark is null)
    or r24, r25
    breq setstack

    ; we are not first, burry a message
    ; for us to find on the other side
    clr r24   ; only zero means we're not first
    push r24

checkstack:
   ; we are not first, check the stack to make
   ; sure that its not getting too big, if it is
   ; we lock down nesting for the time being
   ; we seem to be in an interrupt storm.. if we don't
   ; throttle down, we'll be messing up everyone's business
   in r24, r_SPL
   in r25, r_SPH
   ldd r30, Y+2   ; slimit
   ldd r31, Y+3
   cp  r24, r30
   cpc r25, r31
   ; by using cp, we are subtracting the limit from the
   ; current stack pointer. If the limit is larger than
   ; the SP, the SP has broken the limit, and the carry
   ; flag will be set. Check for carry, instead of "brlt"
   ; brlt would work, unless we had > 32K of RAM
   ; I'd rather not make that assumption.
   brcs intstorm
   rjmp dispatch_isr

intstorm:
   ; we are running out of stack space
   ; we will set the throttle flag and then
   ; implement the autokill mask

   ; apply autokill mask (or)
   ldd r24, Y+7  ; throtl
   ldd r25, Y+8  ; kilmsk

   ; high bit is throttle flag, leave it out
   andi r24, 0x7f
   or r25, r24    ; preserve existing kill flags
   std Y+8, r25

   ; clear hi bit in throttle (disable nesting globally)
   andi r24, 0x7f
   std Y+7, r24
   rjmp dispatch_isr


setstack:
    ; we are in a new ISR, load the current
    ; stack into the mark and set the stack
    ; to the interrupt stack base
    ; also clear throttle flag
    in r24, r_SPL
    in r25, r_SPH
    std Y+4, r24
    std Y+5, r25
    ld  r24, Y
    ldd r25, Y+1
    out r_SPL, r24
    out r_SPH, r25

    ; good to go!
    ; leave a good-news message for our future-self
    ser r24
    push r24    
    rjmp dispatch_isr 


dispatch_isr:


    ; opint.sisr is a special "superpointer" which is a
    ; specified 64 bit unsigned number who's interpretation
    ; is "implementation-specific", in other words: it's
    ; up to us. For AVR5, it's always a __memx pointer
    ; for our implementation, we only support flash pointers
    ; so we will only read in the lowest 2 bytes of sisr
    ; sisr always points to the top of a contiguous
    ; array of SISR files
    
    ; Each SISR file is 5 bytes, and is not aligned at the
    ; word boundry to save space. We need to add the index
    ; multiplied by 5 to select the proper file

    ; save r0 and r1 into r24:25 for mul operation
    movw r24, r0

    ; load operands - r30: irq, r31: x7
    mov r30, r0
    ldi r31, 0x05
    mul r30, r31

    ; now the pointer offset is in r0:r1
    ; and we can load the pointer base to Z
    ; since we assume it's a flash pointer,
    ; and therefore discard the high byte (hh8)
    ldd r30, Y+13  ; sisr
    ldd r31, Y+14
    ; add offset
    add r30, r0
    adc r31, r1
    ; Z is now set to ISR
    ; restore r0:r1
    movw r0, r24

    ; finally, now Z points to the SISR file itself
    ; first check if the flags match the kill mask
    ldd r24, Y+8  ; kilmsk
    lpm r25, Z+   ; flags
    ; quick test of all-kill
    sbrc r24, 7
    rjmp opsec_int_kill 
    and r24, r25
    brne opsec_int_kill  ; kill confirmed

    ; check if we're going to enable nesting now
    ldd r24, Y+7 ; throtl
    ; isolate nesting global disable
    ; (throtl high bit - clear = nesting disabled)
    andi r24, 0x80
    ; for sisr flags, bit 7 set means nesting ok
    and  r24, r25
    ; if both throttle b7 and sisr b7 set, we can
    ; enable nesting
    sbrc r24, 7
    sei

    ; finally, pull the isr and call
    lpm r24, Z+
    lpm r25, Z
    movw r30, r24
    icall
    
    rjmp opsec_int_postmsg
    
opsec_int_kill:
    ; make sure no message is written
    clr r24

opsec_int_postmsg:
    ; according to the specification, the message
    ; should be returned in r24, and r0 should still
    ; have the interrupt number.
    ; it is the responsibility of the ISR to save
    ; all used registers except for r24:26, 28-31

   
    push 20
    push 21

    ldi r30, lo8(opint)
    ldi r31, hi8(opint)
    
    ; if r24 is zero, we dont post any message
    tst r24
    breq 2f
   

    cli

    ; Now we need to quickly check of our interrupt
    ; is the "designated timer", and if so, look for
    ; a "1" message, meaning to set the preemption

    ; the designated timer is a uint16. If you have more than
    ; 4 billion interrupts to handle, I'm so sorry
    ; in our case, we only need to pay attention to the LSB
    ldd r28, Z+21  ; opint.dtimer
    cpse r28, r0
    rjmp 1f

    ; this is the interrupt handler!
    ; check if r24 is 1, if so, we set the
    ; preemption flag
    cpi r24, 0x01
    brne 1f
    
    ser r24
    std Z+6, r24  ; sigpre
    ; no need to process messages
    rjmp 2f

1:              

    ldd r28, Z+9  ; msg_q
    ldd r29, Z+10
    ldd r20, Z+11 ; msg_max
    ldd r21, Z+12

    ; check if msg queue is at end
    ; it shouldn't ever get to that.. but if
    ; it does.. we can't do anything, so we just gotta
    ; move on
    cp r20, r28
    cpc r21, r29
    breq 2f

    ; write out the message
    clr r25
    st Y+, r0   ; irq (uint16) LSB
    st Y+, r25   ; MSB
    st Y+, r24  ; message value
    
    ; write advanced pointer back out
    std Z+9,  r28 ; msq_q
    std Z+10, r29
    

        
1:
    sei
    ; last but not least
    ; we need to find out if we are the last
    ; one out. Basically, when we pop r24 the
    ; first time, if its not zero, we're #1
    ; we do that in case something gets screwed up
    ; by one of the ISRs, we will still re-set the stack
    ; instead of having the preemption timer totally
    ; stop working. an accidental zero here is a bit more
    ; unlikely
    pop r21
    pop r20
    pop r24
    tst r24
    brne 2f
    ; not first, close out
    rjmp opsec_int_closeout

2:
    ; We're "first"
    ; we need reset the stack pointer to smark
    ; and then clear it, before we can pop the rest

    ldd r24, Z+4  ; smark
    ldd r25, Z+5

    cli

    out r_SPL, r24
    out r_SPH, r25
    clr r24
    std Z+4, r24  ; smark
    std Z+5, r24
    
    ; now we check for a preemption signal
    ; for it to count, we need a full signal
    ; that means 0xff, or else it doesn't count
    ser r24
    ldd r25, Z+6  ; sigpre
    cpse r24, r25
    rjmp opsec_int_closeout


preempt:
    ; so this is the real-deal
    ; we're going to need a *full* context save

    ; clear-out pre-emption signal
    clr r24
    std Z+6, r24  ; sigpre

    ; increment p_tick
    ldi r28, lo8(opcon)
    ldi r29, hi8(opcon)

    ldd r24, Y+9  ; opcon.p_tick
    ldd r25, Y+10

    adiw r24, 0x01

    std Y+9, r24  ; opcon.p_tick
    sei
    std Y+10, r25


    ; save the context that we haven't saved already
    push r1
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r26
    push r27
    
    ; call xs!
    rcall opsec_xs

    ;restore context
    pop r27
    pop r26
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1

    ; we still need to clean up "our" part
opsec_int_closeout:
    sei
    pop r31
    pop r30
    pop r29
    pop r28
    pop r0
    pop r25
    out r_SREG, r25
    pop r25
    pop r24
    reti


