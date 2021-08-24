;
; SISR Management
;

;
; .office uint16_t
; .flags  uint8_t
; .isr    (*)void (2 bytes)
;

.globl atria_intr_sisr
.globl intr_sisr

.text

; intr_trap:
;    clr r24
;    ret
    
;
; extern void atria_intr_sisr ( uint64_t * sisr )
;
; sisr: r25:r24
;
atria_intr_sisr:
   ; Basically, we just return the byte address of
   ; the sisr table in a 64-bit number

   push r1
   push r24
   push r25
   push r28
   push r29

   movw r28, r24

   ; -- NOTE --
   ; ATRIA, during loading, re-links
   ; the following ldi symbols with the
   ; actual byte address, so that we
   ; don't need to always do the conversion
   ; every time.
   ; So once loaded here, it's ready for lpm
   ldi r24, lo8(intr_sisr)
   ldi r25, hi8(intr_sisr)

   st Y+, r24
   st Y+, r25

   ; fill in the rest with zeros
   clr r1
   clr r24
   ldi r25, 0x06
1:
   st Y+, r1

   inc r24
   cpse r24, r25
   rjmp 1b

   pop r29
   pop r28
   pop r25
   pop r24
   pop r1
   ret
    
    
