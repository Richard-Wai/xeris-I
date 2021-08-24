; kick into main in absence of start-up files
.globl main
.globl __do_copy_data
.text
.section .init0
__do_copy_data:
rjmp main
