.include "assembly-constants.h" 

.global stack_top
.global main
.global Vector_table_el3

.equ PSTATE_M_EL2, 0b1001 
.equ SCR_RW, 1<<10


.section .text
.global _Reset
_Reset:
    adrp x0, stack_top
    mov sp, x0 
    b main


    
