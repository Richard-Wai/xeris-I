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
; XERIS Interface Group
;
; i/man%
; Human Relations Secretariat
;
; internal assistants
; <man/man.h>
;
; For AVR5 Architecture (Flash assumed < 64 KB)
;

#include <man/internal.h>

; Helper sub-programs, from <man/man.h>
.globl ld_man
.globl ld_soses
.globl memcp

; ROM string index roots
.globl rom_stand   ; MAN% specific strings
.globl rom_soses   ; SOSES strings

.text
;
; void ld_man ( uint16_t idx, unsigned char * dst, uint16_t len )
;
; Load a string from the provided index value from rom_stand,
; up to the maximum length provided by len.
;
; uses internal seek_rom, load_rom to seek the offset
; and load to the pointer
;
; idx: r24:25
; dst: r22:23
; len: r20:21
;
ld_man:
   ; push the clobbers and load the stand root
   ; into the Z pointer, then let-er rip!
   push r1
   push r6
   push r7
   push r16
   push r17
   push r18
   push r19
   push r28
   push r29
   push r30
   push r31
   clr r1

   ; load rom base
   ldi r30, lo8(rom_stand)
   ldi r31, hi8(rom_stand)

   ; set first offset to base
   movw r6, r30
   
   rcall seek_rom;

   ; did it work-out?
   cpse r30, r1
   rjmp 1f
   cpse r31, r1
   rjmp 1f

   ; no, it didnt. So we set the first byte in the
   ; destination to zero
   st Y, r1
   rjmp 2f

1:
   ; ok, load it
   movw r28, r22
   rcall rom_load

2:  
   pop r31
   pop r30
   pop r29
   pop r28
   pop r19
   pop r18
   pop r17
   pop r16
   pop r7
   pop r6
   pop r1
   ret

;
; void ld_soses ( uint16_t spec, uint16_t code,
;                 unsigned char * dst, uint16_t len )
;
; Load a string from the provided index value from rom_stand,
; up to the maximum length provided by len.
;
; uses internal seek_rom, load_rom to seek the offset
; and load to the pointer
;
; spec: r24:25
; code: r22:23
; dst : r20:21
; len : r18:19
;
ld_soses:
   ; push the clobbers and load the stand root
   ; into the Z pointer, then let-er rip!
   push r1
   push r6
   push r7
   push r16
   push r17
   push r18
   push r19
   push r28
   push r29
   push r30
   push r31
   clr r1

   ; load desintation pointer to Y
   movw r28, r20

   ; save length from clobber
   movw r20, r18

   ; load rom base
   ldi r30, lo8(rom_soses)
   ldi r31, hi8(rom_soses)

   ; set first offset to base
   movw r6, r30

   ; spec
   rcall seek_rom;
   ; did it work-out?
   cp r30, r1
   breq 1f
   cp r31, r1
   breq 1f

   ; code
   movw r24, r22
   rcall seek_rom;
   ; did it work-out?
   cpse r30, r1
   rjmp 2f
   cpse r31, r1
   rjmp 2f
   
   ; no, it didnt. So we set the first byte in the
   ; destination to zero
1:
   st Y, r1
   rjmp 3f

2:
   ; ok, load it
   rcall rom_load

3:
   pop r31
   pop r30
   pop r29
   pop r28
   pop r19
   pop r18
   pop r17
   pop r16
   pop r7
   pop r6
   pop r1
   ret


;
; seek_rom
; given a 16-bit index value and the root of a tree,
; seek_rom returns the address of the provided index,
; or zero is none
;
; r1 is expected to be zero
;
; clobbers:
; r16:r17
; r18:r19
;
; r6:7  base address (for spec/code pairs on second round)
; r24:25 index
; r30:31 initialized address (base or second round result)
;
seek_rom:
   ; start by setting-up alpha
   mov r16, r25
   ldi r17, 0x01  ; phase 1 = alpha, 
   andi r16, 0xf0
   swap r16

1:
   lsl r16         ; we need to access a word
   add r30, r16
   adc r31, r1

   ; see what we got.
   lpm r18, Z+
   lpm r19, Z
   cpse r18, r1
   rjmp 2f
   cpse r19, r1
   rjmp 2f

   ; no dice
   clr r30
   clr r31
   ret

2:
   ; from new cycle, we will start back from
   ; the base of the entire index
   movw r30, r6
   ; The loaded offset is in words
   ; so we need to multiply it by 2 first
   lsl r18
   rol r19
   add r30, r18
   adc r31, r19

   ; get next, unless that was gamma
   cpi r17, 0x04
   breq 6f

   ; set up the index properly
   inc r17

   cpi r17, 0x02
   breq 3f
   cpi r17, 0x03
   breq 4f
   cpi r17, 0x04
   breq 5f

3:
   ; beta
   mov r16, r25
   andi r16, 0x0f
   rjmp 1b

4:
   ; delta
   mov r16, r24
   andi r16, 0xf0
   swap r16
   rjmp 1b
   
5:
   ; gamma
   mov r16, r24
   andi r16, 0x0f
   rjmp 1b

6:
   ret
    

;
; load_rom
;
; loads from Z to Y for length bytes
; ensures that the string is capped with
; a zero, or else that Y+(len-1) is zero
;
; r1 assumed to be zero
;
; clobbers:
;   r16:17
;   r24:25
;
; r20:r21 - len
; r28:r29 - destination address
; r30:r31 - source address
;
rom_load:
   ; check for zero length or null
   ; destination
   cpse r20, r1
   rjmp 1f
   cpse r21, r1
   rjmp 1f
   ret

1:
   cpse r28, r1
   rjmp 2f
   cpse r29, r1
   rjmp 2f
   ret

2:
   ; set up the counter
   clr r24
   clr r25

3:
   lpm r16, Z+
   st Y+, r16

   adiw r24, 0x01
   cp r24, r20
   cpc r25, r21
   breq 4f

   ; was that a null terminator?
   cpse r16, r1
   rjmp 3b
   ret

4:
   ; this means we hit the max
   ; ensure that the last one is
   ; a zero in any case
   st -Y, r1
   ret


;
; void memcp ( void * src, void * dst, uint16_t len )
;
; src: r25:r24
; dst: r23:r22
; len: r21:r20
;

memcp:
    push r28
    push r29
    push r30
    push r31

    ; Y = src
    ; Z = dst
    movw r28, r24
    movw r30, r22

    clr r24
    clr r25

1:
    adiw r24, 0x01
    ldd r22, Y+
    std Z+, r22

    cpse r20, r24
    rjmp 1b
    cpse r21, r25
    rjmp 1b


    ; that's it!
    pop r31
    pop r30
    pop r29
    pop r28
    ret


    

    

