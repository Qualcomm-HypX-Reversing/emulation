.include "assembly-constants.h"



.section .text
.global print_cpu_state
.global handle_smc

.macro push_regs

stp x0, x1, [sp]
stp x2, x3, [sp, 0x10]
stp x4, x5, [sp, 0x20]
stp x6, x7, [sp, 0x30]
stp x8, x9, [sp, 0x40]
stp x10, x11, [sp, 0x50]
stp x12, x13, [sp, 0x60]
stp x14, x15, [sp, 0x70]
stp x16, x17, [sp, 0x80]
stp x18, x19, [sp, 0x90]
stp x20, x21, [sp, 0xa0]
stp x22, x23, [sp, 0xb0]
stp x24, x25, [sp, 0xc0]
stp x26, x27, [sp, 0xd0]
stp x28, x29, [sp, 0xe0]
mrs x8, elr_el3
stp x30, x8, [sp, 0xf0]
mrs x8, spsr_el3
mrs x9, esr_el3
stp x8, x9, [sp,0x100] //stored from sp+0x100-sp+0x10f

.endm

.macro default_handler //the default handler will NEVER RETURN

push_regs

mov x0, sp
bl print_cpu_state
b . //if we've reached this state then something has probably gone wrong
.endm

.macro smc_handler

push_regs
mov x0, sp
bl handle_smc


//only x0-x3 can be clobbered
ldp x4, x5, [sp, 0x20]
ldp x6, x7, [sp, 0x30]
ldp x8, x9, [sp, 0x40]
ldp x10, x11, [sp, 0x50]
ldp x12, x13, [sp, 0x60]
ldp x14, x15, [sp, 0x70]
ldp x16, x17, [sp, 0x80]
ldp x18, x19, [sp, 0x90]
ldp x20, x21, [sp, 0xa0]
ldp x22, x23, [sp, 0xb0]
ldp x24, x25, [sp, 0xc0]
ldp x26, x27, [sp, 0xd0]
ldp x28, x29, [sp, 0xe0]
ldr x30, [sp, 0xf0]
eret


.endm



// Typical exception vector table code.
.global Vector_table_el3
.balign 0x1000
Vector_table_el3:
curr_el_sp0_sync:
default_handler
                         
.balign 0x80
curr_el_sp0_irq:  
default_handler
                         
.balign 0x80
curr_el_sp0_fiq:  
default_handler        
                         
.balign 0x80
curr_el_sp0_serror:  
default_handler      
                         
.balign 0x80
curr_el_spx_sync:  
default_handler       

.balign 0x80
curr_el_spx_irq:  
default_handler        

.balign 0x80
curr_el_spx_fiq:    
default_handler      

.balign 0x80
curr_el_spx_serror:  
default_handler      

 .balign 0x80
lower_el_aarch64_sync: 
smc_handler  

.balign 0x80
lower_el_aarch64_irq:  
default_handler    

.balign 0x80
lower_el_aarch64_fiq: 
default_handler

.balign 0x80
lower_el_aarch64_serror: 
default_handler

.balign 0x80
lower_el_aarch32_sync: 
default_handler 

.balign 0x80
lower_el_aarch32_irq:    
default_handler 

.balign 0x80
lower_el_aarch32_fiq:    
default_handler

.balign 0x80
lower_el_aarch32_serror: 
default_handler   
