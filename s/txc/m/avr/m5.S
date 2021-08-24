; ensure vanilla linker script can build us

.globl main
.globl txc_mcpy

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
; void txc_mcpy ( xst_std * src, xst_std * dst )
;
; copy a XERIS Standard ticket from src to dst
;
; src:  r25:r24
; dst:  r23:r22
;
txc_mcpy:
    push r28
    push r29
    push r30
    push r31

    ; source in Y, destination in Z
    movw r28, r24
    movw r30, r22

    ldi r24, 0x0a  ; XERIS Standard tickets are 10 bytes
    clr r25
1:
    inc r25
    ld r22, Y+
    st Z+, r22

    cpse r24, r25
    rjmp 1b

    pop r31
    pop r30
    pop r29
    pop r28
    ret
    

     