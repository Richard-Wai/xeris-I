; KO any start-up files
.globl main
.globl __do_clear_bss
.text
.section .init0
__do_clear_bss:
rjmp main
   
