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
; assist.S
; Assistant proceedures defined in <siplex/siplex.h>
;

#include <sys/m.h>

.text

;.globl rom_load  ** DEPRECIATED
.globl mem_copy
.globl put_shot
.globl pop_shot

.globl a8
.globl a16

.globl int_ena
.globl int_dis
.globl int_swmq

; **DEPRECIATED**
;
; void rom_load ( const __memx void * ld, void * st, uint16_t siz )
;
; load a contiguous section of data from the specificed flash
; space, to the specified RAM space
;
; ld: (r25):r24:r23:r22
; st: r21:r20
; sz: r19:r18
; 
;rom_load:
;    ; confirm that ld is pointing to a rom location
;    ; (most singificant bit of 24-bit memx should be clear)
;    sbrc r24, 7
;    ret
;
;    push r1
;    push r28
;    push r29
;    push r30
;    push r31

;    clr r1



;#if defined (r_RAMPZ)
    ; push RAMPZ
;    in r28, r_RAMPZ
;    push r28

    ; load new value
;    out r_RAMPZ, r24
;#endif
    ; load Z pointer with load (rom) address
;    movw r30, r22
    ; load Y pointer with store (ram) address
;    movw r28, r20

    ; initialize r24:r25 as counter
;    movw r24, r18

;1:
;    sbiw r24, 0x01

;#if defined (r_RAMPZ)
;    elpm r20, Z+
;#else
;    lpm r20, Z+
;#endif        

;    st Y+, r20

;    cpse r24, r1
;    rjmp 1b
;    cpse r25, r1
;    rjmp 1b


;#if defined (r_RAMPZ)
    ; pop RAMPZ
;    pop r28
;    out r_RAMPZ, r28
;#endif

;pop r31
;pop r30
;pop r29
;pop r28
;pop r1
;ret        

;
; void mem_copy ( void * original, void * copy, uint16_t siz )
;
; Complete copy of XERIS Dispatch Case File
;
; original: r25:r24
; copy    : r23:r22
; siz     : r21:r20
;
mem_copy:
    push r1
    push r28
    push r29
    push r30
    push r31
    clr r1

    ; original in Y, copy in Z
    movw r28, r24
    movw r30, r22

    ; use r25:r24 as counter
    movw r24, r20

1:
    sbiw r24, 0x01

    ld r22, Y+
    st Z+, r22

    cpse r24, r1
    rjmp 1b
    cpse r25, r1
    rjmp 1b

    pop r31
    pop r30
    pop r29
    pop r28
    pop r1
    ret

;
; volatile struct shotchain * pop_shot ( volatile struct shotqueue *source )
;
; ATOMIC
; Pop sabbot from top of queue, adjust head
;
; source: r25:r24
; ret   : r25:r24
;

pop_shot:
    push r1
    push r2
    push r28
    push r29
    push r30
    push r31

    clr r1

    ; load source pointer into Y
    movw r28, r24

    ; go critical
    in r2, r_SREG
    cli

    ; act fast
    ldd r30, Y+1 ; shotchain * head
    ldd r31, Y+2

    ; check for null
    cp r30, r1
    cpc r31, r1
    breq 1f  ; null

    ; load in next and set head to it
    ldd r24, Z+2  ; shotchain.next
    ldd r25, Z+3
    ; write out new head
    std Y+1, r24
    std Y+2, r25

    ; check for null
    ; needed to maintain consistency
    ; if head is now null, tail needs to be null
    cp r24, r1
    cpc r25, r1
    breq 2f ; null

    ; normal return
    out r_SREG, r2
    movw r24, r30
    rjmp 3f

1:
    ; head is null, make sure tail is
    ; null and return null
    std Y+3, r1  ; shotqueue.tail
    out r_SREG, r2
    std Y+4, r1
    clr r24
    clr r25
    rjmp 3f

2:
    ; head->next is null, make sure tail is null
    ; return head
    std Y+3, r1  ; shotqueue.tail
    out r_SREG, r2
    std Y+4, r1
    movw r24, r30
    rjmp 3f

3:
    ; clean-up and return
    pop r31
    pop r30
    pop r29
    pop r28
    pop r2
    pop r1
    ret
    
;
; void put_shot ( volatile shotqueue * target, volatile shotchain * sabot )
;
; ATOMIC
; attach new shotchain link (sabot) onto the target queue
;
; target: r25:r24
; sabot : r23:r22
;

put_shot:
    push r1
    push r2
    push r28
    push r29
    push r30
    push r31

    clr r1

    ; load pointer to target queue in Y
    movw r28, r24

    ; ensure sabnot->next is null
    movw r30, r22
    std Z+2, r1  ; shotchain.next
    std Z+3, r1

    ; go critical
    in r2, r_SREG
    cli

    ; load target->tail
    ldd r24, Y+3
    ldd r25, Y+4

    ; check if null
    cp r24, r1
    cpc r25, r1
    breq 1f  ; null

    ; set target->tail to point to sabot
    std Y+3, r22 ; 
    std Y+4, r23

    ; point Z at old tail
    movw r30, r24
    ; link old tail (.next) to new tail (sabot)
    std Z+2, r22
    out r_SREG, r2
    std Z+3, r23

    rjmp 2f

1:
    ; tail is null
    ; set head and tail to sabot
    std Y+1, r22
    std Y+2, r23
    std Y+3, r22
    out r_SREG, r2
    std Y+4, r23

2:
    ; clean-up
    pop r31
    pop r30
    pop r29
    pop r28
    pop r2
    pop r1
    ret


    
        
;
; uint8_t a8 ( volatile void * target, uint8_t val )
;
; Generic atomic get set swap operation (8-bit)
;
; target = r25:r24
; val    = r22
;
; ret    = r24
;

a8:
    push r1
    push r28
    push r29

    movw r28, r24
    
    in r1, r_SREG
    cli
    ld r24, Y
    out r_SREG, r1
    st Y, r22

    pop r29
    pop r28
    pop r1
    ret

;
; uint16_t a16 ( volatile void * target, uint16_t val )
;
; Generic atomic get set swap operation (8-bit)
;
; target = r25:r24
; val    = r23:r22
;
; ret    = r25:r24
;

a16:
    push r1
    push r28
    push r29

    movw r28, r24

    in r1, r_SREG
    cli
    ld r24, Y
    ldd r25, Y+1
    st Y, r24
    out r_SREG, r1
    std Y+1, r25

    pop r29
    pop r28
    pop r1
    ret


    
;
; void int_ena ( void )
;
; enable interrupts
;
int_ena:
    sei
    ret

;
; void int_dis ( void )
;
; disable interrupts
;
int_dis:
    cli
    ret

;  
; extern void int_swmq
; ( volatile struct siplex_imsg * volatile * old_mark,
;   volatile struct siplex_imsg            * new_mark,
;   volatile struct siplex_imsg * volatile * old_max,
;   volatile struct siplex_imsg            * new_max );
;
; Switch interrupt message queue
; (atomic)
;
; old_mark **: r25:r24
; new_mark * : r23:r22
; old_max  **: r21:r20
; new_max  * : r19:r18 
;

int_swmq:
    ; pretty simple, just write out
    ; new mark to locaation of old mark
    ; and same with max, but done atomically

    ; Y: old_mark **
    ; Z: old_max  **

    push r0
    push r28
    push r29
    push r30
    push r31

    movw r28, r24  ; Y
    movw r30, r20  ; Z

    ; go critical
    in r0, r_SREG
    cli

    st Y+, r22
    st Y, r23
    st Z+, r18
    out r_SREG, r0
    st Z, r19

    pop r31
    pop r30
    pop r29
    pop r28
    pop r0
    ret
    

