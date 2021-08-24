; ensure vanilla linker script can build us

.globl main
.globl smcpy

.text
.section .vectors
vectors:
    jmp main
.section .init0
nop
.section .init2
nop
.section .init4
nop
.section .init9
nop

;
; void smcpy ( void * src, void * dst, uint8_t size )
;
; copy small (<255) byte portion of ram
;
; src:  r25:r24
; dst:  r23:r22
; size: r20
;
smcpy:
    push r28
    push r29
    push r30
    push r31

    ; Y = src
    ; Z = dst

    movw r28, r24
    movw r30, r22

    clr r22

1:
    inc r22
    ld r23, Y+
    st Z+, r23
    cpse r20, r22
    rjmp 1b

    pop r31
    pop r30
    pop r29
    pop r28
    ret

    


