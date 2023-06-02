.global stack_top
.global main

.global _Reset
_Reset:
    adrp x0, stack_top
    mov sp, x0 
    b main

.global goto_hyp
goto_hyp:
    b .

    
