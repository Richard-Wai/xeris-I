#include <sys/m.h>

.text

; Our responsiblilties
.globl ulif_if_init
.globl ulif_if_rels
.globl ulif_if_txchar
.globl ulif_if_rxchar

; ULIFs are usually hard-coded. We have a config block here
; to allow non-recompile config changes, but it's pretty particular
; and not standardized (yet)

.globl uart_config_baud
; Set
uart_config:
.byte lo8(__XERIS_BUILD_SFSYS)
.byte hi8(__XERIS_BUILD_SFSYS)
.byte lo8(__XERIS_BUILD_SFSYS >> 16)
.byte hi8(__XERIS_BUILD_SFSYS >> 16)
.word __FLATLINE_ULIF_AVR5_UART_BAUD
.byte __FLATLINE_ULIF_AVR5_UART_DATA
.byte __FLATLINE_ULIF_AVR5_UART_PAST


;
; This will let us properly calculate the "ubrr" value
; for the given BAUD and clock
;
; r25:r22 - clock rate (Hz)
; r21:r20 - baud
;
; return: r25:r24 - ubrr
;
uart_ubrr:

;r25 = clock MSB
;r22 = clock LSB

;r21 = baud MSB
;r20 = baud LSB

;r0 = scratch
;r1 = scratch
;r16 = remainder
;r17 = scatch
;r19 = multiplyer / counter
;r18 = multiplyer / counter

push r0
push r1
push r16
push r17
push r18
push r19

ldi r18,0x10	; multiply by 16 = << 4 if high nibble = 0

mov r16,r22
andi r16,0x0f	; clear high nibble of remainder

lsr r22
lsr r22
lsr r22
lsr r22

mov r17,r23
andi r17,0x0f
mul r17,r18
or r22, r0

lsr r23
lsr r23
lsr r23
lsr r23

mov r17, r24
andi r17,0x0f
mul r17,r18
or r23, r0

lsr r24
lsr r24
lsr r24
lsr r24

mov r17, r25
andi r17,0x0f
mul r17,r18
or r24, r0

lsr r25
lsr r25
lsr r25
lsr r25

; Done, we should now have r2:r5 / 16
; check remainder, if > 8, lets add one
ldi r17, 0x01
mov r0, r17
clr r1
cpi r16, 0x09
brlo 1f
; greater than 0x8, so we round-up
add r22, r0
adc r23, r1
adc r24, r1
adc r25, r1

1:
; long hand devision basically!
; r6:r7 = counter
clr r18
clr r19


2:		; see how many times we can fit the baud into clk/16
sub r22, r20
sbc r23, r21
sbc r24, r1
sbc r25, r1
brcs 3f		; if carry set, we ran out!
inc r18
cpse r18, r1	; counter low wrap-around
rjmp 2b
inc r19
cpse r19, r1 ; counter high wrap! overflowwwwwwwwwww
rjmp 2b
clr r18		; notify handler of overflow and exit
clr r19
jmp 4f

3:
; last step, subtract one!
sub r18, r0
sbc r19, r1

4:
; send back the ubrr value as a return
movw r24, r18

5:
pop r19
pop r18
pop r17
pop r16
pop r1
pop r0

ret


;
; extern uint16_t      ulif_if_init   ( void );
;
; Initialize the interface, and return a byte pair
; indicating the TX lines (MSB0) and RX lines (LSB)
;
ulif_if_init:
   push r1
   push r22
   push r23
   push r30
   push r31

   ; First step is to calculate the ubrr value
   ; to do that, we need to load the clock value
   ; as well as the

   ; Set Z pointer to config block
   ldi r30, lo8(uart_config)
   ldi r31, hi8(uart_config)

   ; We will load in directly to the appropriate
   ; registers for the call to uart_ubrr

   ; system clock
   lpm r22, Z+
   lpm r23, Z+
   lpm r24, Z+
   lpm r25, Z+

   ; baud
   lpm r20, Z+
   lpm r21, Z+  ; now points at data bits

   call uart_ubrr
   ; r25:24 has ubrr

   ; Start by clearig the status/control registers
   clr r1
   sts r_ucsra, r1
   sts r_ucsrb, r1
   sts r_ucsrc, r1

   ; Start setting up by first writing out the ubrr
   sts r_ubrrl, r24
   sts r_ubrrh, r25

   ; we will now use r24 to set up ucsrb
   clr r24

   ; Now, set data bits. We limit between 5 and 9
   ; so we check this, and if it's out of range,
   ; we settle with the save default of 8
   lpm r22, Z+
   cpi r22, 10
   brlo 1f
   ldi r22, 8
1:
   cpi r22, 5
   brsh 1f
   ldi r22, 8
1:
   ; Now we need to convert it to a format that we can
   ; write to the register with. This is a bit weird.
   ; basically if we subtract 5, it works up to 9
   ; in which case we need to set it to 0b111
   subi r22, 5

   ; check if bit 2 is set, in which case, 1, 0 need to
   ; also be set
   sbrc r22, 3
   ori r22, 3

   ; Now we can prepare ucsrb (r24) with the high bit of the character
   ; size, if any
   sbrc r22, 3
   ldi r24, b_ucsrb_ucsz2

   ; Then we can use r22 as the ultimate ucsrc to be
   ; written. We need to start by shifting left 1,
   ; as byte size are in bits 2:1
   lsl r22

   ; set up stop bits (1 or zero)
   ; The stop/parity byte in the control block
   ; is made of two nibbles. The low nibble is the stop
   ; bit setting. 1 = 1, 2 = 2
   lpm r23, Z
   sbrc r23, 0
   ori r22, b_ucsrc_usbs

   ; Set parity
   ; The upper nibble goes like this
   ; 0 = no parity, odd = odd, even = even
   swap r23
   andi r23, 0x0f
   cpi r23, 0x00
   breq 2f ; no parity, do nothing
   
   ; odd or even?
   sbrs r23, 0
   rjmp 1f ; odd
   ori r22, 0b00100000  ; even
   rjmp 2f
1:
   ori r22, 0b00110000  ; odd
2:

   ; Good to write out ucrsc
   sts r_ucsrc, r22

   ; That's basically it. Now we can enable the
   ; TX/RX
   ; we add in the tx/rx enable bits into the prepared
   ; value in r24, and let 'er rip
   ori r24, 0b00011000
   sts r_ucsrb, r24

   ; Now we hard-wire one tx and one rx here, so just
   ; set the return to 0x0101
   ldi r24, 0x01 ; 1 rx
   ldi r25, 0x01 ; 1 tx

   pop r31
   pop r30
   pop r23
   pop r22
   pop r1
   ret


;
; Release
; Basicaly shut-down and clear-out.
; Should prepare for a new driver to re-initialize
;
; extern void ulif_if_rels ( void )
;
ulif_if_rels:
   push r1
   clr r1

   sts r_ucsrb, r1  ; disables the transmitters
   sts r_ucsra, r1
   sts r_ucsrc, r1
   sts r_ubrrl, r1
   sts r_ubrrh, r1

   pop r1
   ret

;
; Transmit character
;
; extern void ulif_if_txchar ( unsigned char c )
;
; char: r24 - the char
;
ulif_if_txchar:
   push r25

1:
   ; check for a free transmitter before sending!
   lds r25, r_ucsra
   sbrs r25, 5  ; udre
   rjmp 1b

   ; good to go!
   sts r_udr, r24

   pop r25
   ret


;
; Receive character
;
; extern unsigned char ulif_if_rxchar ( void );
;
; ret: r24 - the char
;
ulif_if_rxchar:
   push r25

   ; wait for one to be available
1:
   lds r25, r_ucsra
   sbrs r25, 7 ;rxc
   rjmp 1b

   lds r24, r_udr

   pop r25
   ret

   
   
   
   
   
   
   

    
