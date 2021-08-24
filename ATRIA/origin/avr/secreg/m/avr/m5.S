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
; ATRIA%
; Authorized Track Registrar and Integration Agent
;
; SECREG$
; Secretariat Registrar
;
; Machine-dependent Components
;

;
; atria_secreg_pullsrf
; atria_secreg_pullsoc
;

#include <sys/m.h>
    
#define SIZEOF_SRF 0x1d
#define SIZEOF_SOC 0x0c

#define SRF_SOC_OFFSET  0x11

.globl atria_secreg_pullsrf
.globl atria_secreg_pullsoc
.globl atria_secreg_nextsid

; location of Alpha Registrar Table
.globl atria_secreg_alpha

.text

;
; private function to pull the actual
; "hard-copy" Registration File pointer
; (for avr, this means in flash)
;
; SID passed in on r25:r24
; PROGMEM pointer returned on r30:r31 (Z)
;

index_file:
   ; SID in on r24:r25
   ; PROGMEM pointer out on r28:r29 (Y)
   push r1   ; zero
   push r2   ; cycle (0 = alpha, 3 = gamma)

   push r20  ; ALPHA
   push r21  ; BETA
   push r22  ; DELTA
   push r23  ; GAMMA
   
   push r28
   push r29

   clr r1
   clr r2

   ; so it works like this: the index is broken up into 4 blocks,
   ; each with _maximum_ of 16 pointers. The alpha block pointers
   ; point to a beta block, and so-on. Each block has a 2-byte 
   ; header, both bytes must be zero for the block to be considered
   ; active/normal. If either byte is not zero, the block is decommissioned
   ; the first pointer in the index should point to the active block

   ; the high byte is the maximum available index for that block.
   ; Blank indexes have a null pointer, and any failure herein
   ; will always pass back a null pointer


   ; First step is to break down the block cells from the SID
   ; alpha
   mov r20, r25
   andi r20, 0xf0
   swap r20

   ; beta
   mov r21, r25
   andi r21, 0x0f

   ; delta
   mov r22, r24
   andi r22, 0xf0
   swap r22
   
   ; gamma
   mov r23, r24
   andi r23, 0x0f

   
   ; Load the pre-configured pointer to the
   ; alpha block (atria_secreg_alpha)

   ldi r30, lo8(atria_secreg_alpha)
   ldi r31, hi8(atria_secreg_alpha)

   ; note in next loop section, r20 is always the "working"
   ; register. Logic after the loop will load the next block
   ; index into r20 as appropriate for each "phase" (r2)

index_seek:
   ; check is header active (low + hi both byte clear)
   lpm r28, Z+
   cpse r28, r1
   rjmp bad_header
   
   lpm r28, Z+
   cpse r28, r1
   rjmp bad_header


   ; Z now points to the first (low) byte of index 0 on the
   ; block. Pointers are 2 bytes, so we need to multiply our
   ; index offset by 2 before adding it to Z
   ; we know that the index max must be 0x0f, so we can safely
   ; do one logical shift, then we're golden
   lsl r20
   add r30, r20
   adc r31, r1

   ; Load in the pointer at the index
   lpm r28, Z+
   lpm r29, Z

   ; check for "dead-end"
   cpse r28, r1
   rjmp 1f
   cpse r29, r1
   rjmp 1f
   ; pointer is null, which means dead-end
   rjmp bad_index

1:
   ; set Z to the new pointer
   movw r30, r28

   ; now we check the phase, to see what to do with the Y
   ; pointer

   inc r2
   mov r28, r2

   mov r20, r21     ; load next "index" value (beta here)
   cpi r28, 0x01    ; finished alpha, do beta
   breq index_seek
   
   mov r20, r22
   cpi r28, 0x02    ; finished beta, do delta
   breq index_seek

   mov r20, r23
   cpi r28, 0x03    ; finished delta, do gamma
   breq index_seek

   ; finished gamma, Z points to the SRF!
ret_index:
   pop r29
   pop r28
   pop r23
   pop r22
   pop r21
   pop r20
   pop r2
   pop r1
   ret

bad_header:
   ; load new pointer and try again
   lpm r28, Z+
   lpm r29, Z
   movw r30, r28
   rjmp index_seek

bad_index:
   ; no such index!
   ; set return to zero
   clr r30
   clr r31
   rjmp ret_index
   
   
;
; uint8_t atria_secreg_pullsrf ( uint16_t sid, struct atria_srf * srcopy )
;
; Retreives the SRF "hard-copy" for the provided SID, if possible.
; If unable, returns 0. If able, returns 0xff
;
; sid: r25:r24
; srcopy: r23:r22
;
; ret: r24
;

atria_secreg_pullsrf:
   push r1
   push r28
   push r29
   push r30
   push r31

   ; load destination (copy-out file) into Y pointer
   movw r28, r22

   ; index_file will attempt to put a pointer to the srf
   ; in the Y pointer for us (from the SID already in r25:r24)
   rcall index_file

   ; we will now use r24:r25 as a counter for the copy loop
   clr r24
   ldi r25, SIZEOF_SRF

   ; check for null pointer first
   clr r1
   cpse r30, r1
   rjmp 1f
   cp r31, r1
   breq 2f  ; no good!
   
1: ; copy loop
   inc r24
   lpm r22, Z+
   st Y+, r22

   cpse r24, r25
   rjmp 1b

   ser r24

2:
   pop r31
   pop r30
   pop r29
   pop r28
   pop r1
   ret

;
; uint8_t atria_secreg_pullsoc ( uint16_t sid, struct atria_soc * cert )
;
; Retreives the SOC "hard-copy" for the provided SID, if possible.
; If unable, returns 0. If able, returns 0xff
;
; sid: r25:r24
; cert: r23:r22
;
; ret: r24
;

atria_secreg_pullsoc:
   push r1
   push r28
   push r29
   push r30
   push r31

   ; load destination (copy-out file) into Y pointer
   movw r28, r22

   ; index_file will attempt to put a pointer to the srf
   ; in the Y pointer for us (from the SID already in r25:r24)
   call index_file

   ; we will now use r24:r25 as a counter for the copy loop
   clr r24
   ldi r25, SIZEOF_SOC

   ; check for null pointer first
   clr r1
   cpse r30, r1
   rjmp 1f
   cp r31, r1
   breq 3f  ; no good!

1:
   ; we need to add the offset to SOC to the
   ; Z pointer
   adiw r30, SRF_SOC_OFFSET
   
2: ; copy loop
   inc r24
   lpm r22, Z+
   st Y+, r22

   cpse r24, r25
   rjmp 2b

   ser r24

3:
   pop r31
   pop r30
   pop r29
   pop r28
   pop r1
   ret   

   
;
; uint16_t atria_secreg_nextsid ( uint16_t mark )
;
; Seeks to the next sid following the one indicated
; by mark. Allows fastest traversal of entire registration
; database, rather than trying to pull each file.
;
; The function is circular, sice 0000% (opsec) must exist.
;
;
; mark: r25:r24
;
; ret: r25:r24
;

atria_secreg_nextsid:
   ; Basically it works like this. We take in an existing SID,
   ; increment it by one, then see if we can seek to it.
   ; If the seek fails before we get to the GAMMA table, then we
   ; increment that table and re-try from the start.


   push r1
   push r16
   push r17
   push r18
   push r19
   push r20
   push r21
   push r30
   push r31

   clr r1
   adiw r24, 0x01

1:  ; retry

   ldi r30, lo8(atria_secreg_alpha)
   ldi r31, hi8(atria_secreg_alpha)
   rcall atria_secreg_nextsid_checkblock
   
   ; alpha
   mov r16, r25
   andi r16, 0xf0
   swap r16
   lsl r16
   add r30, r16
   adc r31, r1
   lpm r18, Z+
   lpm r19, Z
   cpse r18, r1
   rjmp 2f      ; ok, we have a beta
   cpse r19, r1
   rjmp 2f

   ; no alpha, start with alpha+1 + 000
   lsr r16
   inc r16
   andi r16, 0x0f ;mask - out any overflow
   swap r16
   mov r25, r16
   clr r24
   cpse r24, r25 ; no need to "check" zero, save some time
   rjmp 1b
   rjmp 8f

2:
   movw r30, r18
   rcall atria_secreg_nextsid_checkblock
   movw r20, r30
   ldi r17, 0x10
3:
   ; how's beta?
   movw r30, r20
   mov r16, r25
   andi r16, 0x0f
   lsl r16
   add r30, r16
   adc r31, r1
   lpm r18, Z+
   lpm r19, Z
   cpse r18, r1
   rjmp 4f
   cpse r19, r1
   rjmp 4f

   ; try next one
   inc r25
   clr r24
   lsr r16
   inc r16
   cpse r16, r17
   rjmp 3b

   ; out of alpha, back to the start,
   ; unless alpha just rolled over too,
   ; in which case we're done (return 0000)
   cpse r25, r24
   rjmp 1b
   rjmp 8f

4:
   ; set up delta
   movw r30, r18
   rcall atria_secreg_nextsid_checkblock
   movw r20, r30
   ldi r17, 0x01
5:
   movw r30, r20
   mov r16, r24
   andi r16, 0xf0
   swap r16
   lsl r16
   add r30, r16
   adc r31, r1
   lpm r18, Z+
   lpm r19, Z
   cpse r18, r1
   rjmp 6f
   cpse r19, r1
   rjmp 6f

   ; try next
   lsr r16
   inc r16
   swap r16
   mov r24, r16
   cpse r16, r17
   rjmp 5b

   ; out of delta back to the start
   ; unless alpha+beta will roll over
   clr r24
   inc r25
   cpse r24, r25
   rjmp 1b
   rjmp 8f

6:
   ; set up gamma
   movw r30, r18
   rcall atria_secreg_nextsid_checkblock
   movw r20, r30
   ldi r17, 0x10
7:
   movw r30, r20
   mov r16, r24
   andi r16, 0x0f
   lsl r16
   add r30, r16
   adc r31, r1
   lpm r18, Z+
   lpm r19, Z
   cpse r18, r1
   rjmp 8f
   cpse r19, r1
   rjmp 8f

   ;try next
   lsr r16
   inc r16
   inc r24
   cpse r16, r17
   rjmp 7b

   ; nope, all out. go back to the start
   ; unless we just hit ffff
   cpse r24, r1
   rjmp 1b
   ; overflow into r25
   inc r25
   cpse r24, 25
   rjmp 1b

8:
   ; clean up
   pop r31
   pop r30
   pop r21
   pop r20
   pop r19
   pop r18
   pop r17
   pop r16
   pop r1
   ret

         
; Private subroutine to check the block
; and seek to new block if the block is marked as bad
; Z pointer should be set to block, r16:17 clobbered
; On return Z points to high byte of index block header
    
atria_secreg_nextsid_checkblock:

    ; First one
    lpm r16, Z+
    cpi r16, 0x00
    brne 1f

    ; Second one
    lpm r16, Z+
    cpi r16, 0x00
    brne 1f

    ; looks good 
    ret
    
1:
   ; ok. so we have a "bad" block.
   ; the rule is we do a best attempt
   ; at retreiving a pointer to the replacement
   ; block. We say best attempt, because there is no room
   ; here for total failure. Best attempt means we try to
   ; see if we get the same pointer in two consecutive tries
   ; or else we keep trying until the end of the block
   ; advance Z to first pointer

   ; remember this is a "privtate" subroutine. We can rely on
   ; r1 being zero

   push r18
   push r20
   push r21
   
   ldi r18, 0x08
2:
   cp r18, r1
   breq 3f ; we gave it our best shot

   lpm r16, Z+
   lpm r17, Z+

   lpm r20, Z+
   lpm r21, Z+

   dec r18

   cp r16, r20
   brne 2b
   cp r17, r21
   brne 2b

3:

   movw r30, r20
   pop r21
   pop r20
   pop r18
   rjmp atria_secreg_nextsid_checkblock

    
   
