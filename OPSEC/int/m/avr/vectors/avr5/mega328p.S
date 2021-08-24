;
; MEGA328p
; Vector table
;

;
; This file is included directly
; from m5.S
;

.globl int0      ; RESET
.globl int1      ; INT0 - External IRQ
.globl int2      ; INT1 - External IRQ
.globl int3      ; PCINT0 - pin change IRQ0
.globl int4      ; PCINT1 - pin change IRQ1
.globl int5      ; PCINT2 - pin change IRQ2
.globl int6      ; WDT - watchdog timeout
.globl int7      ; TC2 Compare A match
.globl int8      ; TC2 Compare B match
.globl int9      ; TC2 Overflow
.globl int10     ; TC1 Capture
.globl int11     ; TC1 Compare A match
.globl int12     ; TC1 Compare B match
.globl int13     ; TC1 Overflow
.globl int14     ; TC0 Compare A match
.globl int15     ; TC0 Compare B match
.globl int16     ; TC0 Overflow
.globl int17     ; SPI Serial Tranfer Complete
.globl int18     ; USART Rx complete
.globl int19     ; USART Data register empty
.globl int20     ; USART Tx complete
.globl int21     ; ADC conversion complete
.globl int22     ; EEPROM ready
.globl int23     ; Analog comparator
.globl int24     ; TWI
.globl int25     ; SPM ready

      
;
; actual map
;
; atmega328 p - each vector is two words
;
.text
.section .vectors
vectors:
    rjmp int0
    nop
    
    rjmp int1
    nop
    
    rjmp int2
    nop
    
    rjmp int3
    nop
    
    rjmp int4
    nop
    
    rjmp int5
    nop
    
    rjmp int6
    nop
    
    rjmp int7
    nop
    
    rjmp int8
    nop
    
    rjmp int9
    nop
    
    rjmp int10
    nop
    
    rjmp int11
    nop
    
    rjmp int12
    nop
    
    rjmp int13
    nop
    
    rjmp int14
    nop
    
    rjmp int15
    nop
    
    rjmp int16
    nop
    
    rjmp int17
    nop
    
    rjmp int18
    nop
    
    rjmp int19
    nop
    
    rjmp int20
    nop
    
    rjmp int21
    nop
    
    rjmp int22
    nop
    
    rjmp int23
    nop
    
    rjmp int24
    nop
    
    rjmp int25
    nop

    ; XERIS syscall, which means us (Central Dispatch Desk)
    rjmp opsec_cdd

.text        
        
; Call map
int0:
    ; device reset
    cli

    ; set stack pointer to end
    ldi r28, lo8(c_RAMEND)
    ldi r29, hi8(c_RAMEND)
    out r_SPH, r29
    out r_SPL, r28
    clr r0
    clr r1
    clr r28
    clr r29
    
    rjmp sys_boot

int1:
    push r24
    ldi r24, 0x01
    rjmp opsec_int_enter

int2:
    push r24
    ldi r24, 0x02
    rjmp opsec_int_enter

int3:
    push r24
    ldi r24, 0x03
    rjmp opsec_int_enter

int4:
    push r24
    ldi r24, 0x04
    rjmp opsec_int_enter

int5:
    push r24
    ldi r24, 0x05
    rjmp opsec_int_enter
     
int6:
    push r24
    ldi r24, 0x06
    rjmp opsec_int_enter

int7:
    push r24
    ldi r24, 0x07
    rjmp opsec_int_enter

int8:
    push r24
    ldi r24, 0x08
    rjmp opsec_int_enter

int9:
    push r24
    ldi r24, 0x09
    rjmp opsec_int_enter

int10:
    push r24
    ldi r24, 0x0a
    rjmp opsec_int_enter

int11:
    push r24
    ldi r24, 0x0b
    rjmp opsec_int_enter

int12:
    push r24
    ldi r24, 0x0c
    rjmp opsec_int_enter

int13:
    push r24
    ldi r24, 0x0d
    rjmp opsec_int_enter

int14:
    push r24
    ldi r24, 0x0e
    rjmp opsec_int_enter

int15:
    push r24
    ldi r24, 0x0f
    rjmp opsec_int_enter

int16:
    push r24
    ldi r24, 0x10
    rjmp opsec_int_enter

int17:
    push r24
    ldi r24, 0x11
    rjmp opsec_int_enter

int18:
    push r24
    ldi r24, 0x12
    rjmp opsec_int_enter

int19:
    push r24
    ldi r24, 0x13
    rjmp opsec_int_enter

int20:
    push r24
    ldi r24, 0x14
    rjmp opsec_int_enter

int21:
    push r24
    ldi r24, 0x15
    rjmp opsec_int_enter

int22:
    push r24
    ldi r24, 0x16
    rjmp opsec_int_enter

int23:
    push r24
    ldi r24, 0x17
    rjmp opsec_int_enter

int24:
    push r24
    ldi r24, 0x18
    rjmp opsec_int_enter

int25:
    push r24
    ldi r24, 0x19
    rjmp opsec_int_enter














