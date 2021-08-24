#include <sys/m.h>

.globl spi_init
.globl spi_send

.text

;
; extern void spi_init ( void )
;
spi_init:
   ; set up port b
   push r16
   push r17

   in r16, r_ddrb

   ; Set ddrs for MOSI and SCK to be outputs,
   ; and clear pin

             ;76543210
   ldi r17, 0b11000011
   and r16, r17
             ;76543210
   ori r16, 0b00101100 ; MOSI + SCK + SS as outputs
   out r_ddrb, r16

   in r16, r_pinb
   and r16, r17
   out r_pinb, r16 ; all of our pins as zero

   ; reset SPI module
   clr r16
   out r_spcr, r16
   out r_spdr, r16
   in r16, r_spdr

   ; Now set up the SPI module
   ldi r16, 0b01010011 ; master, Fosc/128
   out r_spcr, r16

   ; Write fat 0xff just to make sure the
   ; SPIF flag is up correctly
   in r16, r_spdr
   ser r16
   out r_spdr, r16

   ; All set
   pop r17
   pop r16
   ret

;
; extern void spi_send ( xdcf * cf );
;
; cf * r25:r24
spi_send:
   push r1
   push r28
   push r29

   clr r1

   ; cf pointer in Y
   movw r28, r24

   ; seek to 
   adiw r28, 7
    
   ; Set up counter. ticket is 23 bytes
   ldi r24, 10

1:
   ; poll until clear
   in r25, r_spsr
   sbrs r25, 7
   rjmp 1b

   ; OK, write out the next one
   ld r25, Y+
   out r_spdr, r25

   ; decrement counter
   subi r24, 1
   cpse r24, r1
   rjmp 1b

   pop r29
   pop r28
   pop r1
   ret

    
    






