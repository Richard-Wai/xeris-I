; just because of the linker scripts

; dd
.text

.globl siplex
.globl __do_copy_data

.section .init1
        
siplex_kick:
__do_copy_data:
rjmp siplex
