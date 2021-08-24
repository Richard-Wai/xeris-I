; ensure vanilla linker script can build us

.globl main
.globl mclr

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
; void mclr ( void * tgt, uint8_t size )
;
; clear small (<255) byte portion of ram
;
; tgt:  r25:r24
; size: r22
;
mclr:
    push r28
    push r29

    ; Y = tgt

    movw r28, r24

    clr r24
    clr r25

1:
    inc r24
    st Y+, r25
    cpse r22, r24
    rjmp 1b

    pop r29
    pop r28
    ret

    


