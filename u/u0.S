#include <sys/m.h>
.globl main
.globl clight

.section .vectors
vectors:
    jmp main

.text
clight:
    sbi r_ddrb, 0
    sbi r_pinb, 0
    ret

