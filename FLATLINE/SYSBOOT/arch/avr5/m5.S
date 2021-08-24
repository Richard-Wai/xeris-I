#include <sys/m.h>

;FLATLINE%SYSBOOT$ main entry code

.globl __do_copy_data
.globl flatline

.text
.section .init0
vectors:
; this ensures library is eliminated
__do_copy_data:
; Dispatch Desk always goes to virtual address 0 (entry)
clr r1
rjmp flatline

