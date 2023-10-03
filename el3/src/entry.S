.include "assembly-constants.h" 

.global stack_top
.global main
.global Vector_table_el3

.equ PSTATE_M_EL2, 0b1001 
.equ PSTATE_M_EL1, 0b0101
.equ SCR_RW, 1<<10

.section .data
hyp_entry:
    .quad 0x80040000
kernel_entry:
    .quad 0x81000000
hcr:
    .quad 0x84080004


.section .text
.global _Reset
_Reset:
    adrp x0, stack_top
    mov sp, x0 
    adr x0, Vector_table_el3 //load VBAR_table
    msr VBAR_EL3, x0
    mov x0, #1 
    msr SPSel, x0 //use saved sp
    mrs x0, SCR_EL3
    mov x1, #1
    orr x0, x0, #1 //set NS bit
    orr x0, x0, #SCR_RW //set the RW bit
    msr SCR_EL3, x0
    b main


.global start_kernel
start_kernel:
    adr x4, kernel_entry
    ldr x4, [x4]
    msr ELR_EL3, x4
    mov x4, #PSTATE_M_EL1
    msr SPSR_EL3, x4
    eret

.global start_hyp
start_hyp:
    adr x4, hyp_entry //get the address of hyp_entry
    ldr x4, [x4]
    msr ELR_EL3, x4
    mov x4, #PSTATE_M_EL2
    msr SPSR_EL3, x4
#    eor x0, x0, x0
#    eor x1, x1, x1
    eor x2,x2,x2
    eor x3,x3,x3
    sub sp, sp, #CPU_STATE_SIZE //this sets SP_EL3 to CPU_STATE_SIZE. slight optimization so don't have to keep subtracting when we enter
    eret

    
