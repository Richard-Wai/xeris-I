;
; System reference clock
;
; Generates the preeption signal
; for OPSEC%
;

#include <sys/m.h>

.globl clock_init
.globl clock_isr

.text

clock_init:
    ; Set up clock to trip on overflow
    ; and to trigger only the overflow
    ; interrupt.

    ; prescaler will be /64

    push r28
    clr r28

    ; turn of compare matching
    out r_tccr0a, r28

    ; clear timer
    out r_tcnt0, r28

    ; set interrupts for overflow only
    ldi r28, 0x01  ; overflow inerrupt only
    sts r_timsk0, r28

    ; set prescaler (enables timer)
    ldi r28, 0x5 ; (/1024) 
    out r_tccr0b, r28

    pop r28
    ret
       

clock_isr:
    ; actual isr!
    ; all we do is set the message to "1"
    ; This tells OPSEC that we want to trigger
    ; a "tick"
    ldi r24, 0x01
    ret

    
    
    

    
