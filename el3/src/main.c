#include <stdint.h>
#include "printf.h"
#include "cpu_state.h"
#include "smccc.h"
#include "psci.h"
extern void start_hyp();

void print_cpu_state(struct cpu_state* cs){

    printf("========CPU STATE START========\n");
    for(int i = 0; i<GP_REG_NUM; i++){
        printf("x%d: 0x%lx\n", i, cs->x[i]);
    }
    printf("ELR_EL3: 0x%lx\n", cs->elr_el3);
    printf("ESR_EL3: 0x%lx\n", cs->esr_el3);
    printf("SPSR_EL3: 0x%lx\n", cs->spsr_el3);
    printf("=========CPU STATE END=========\n");
}

uint64_t handle_smc(struct cpu_state* cs){
    uint32_t call_code = (uint32_t)cs->x[0]; //the function_code is always 32bits according ARM SMCCC

    print_cpu_state(cs);
    if(!IS_64(call_code)){ 
        printf("SMC call made using 32 bit CC\n");
    }else{
        printf("SMC Call made usin 64 bit CC\n");
    }

    if(IS_FAST_CALL(call_code)){
        printf("SMC is a fast call\n");
    }else{
        printf("SMC is not a fast call\n");
    }

    switch(SERVICE_CALL_NUMBER(call_code)) {
        case 0:
            printf("Architecture Call\n");
            break;
        case 1: 
            printf("CPU Service Call\n");
            break;
        case 2:
            printf("Silicon Partner Call\n");
            break;
        case 3: 
            printf("OEM Service Calls\n");
            break;
        case 4: 
            printf("Standard Secure Service Calls\n");
            if(call_code == 0x84000000){
                printf("Returning PSCI Version\n");
                return PSCI_VERSION;
            }
            break;
        case 5:
            printf("Standard Hypervisor Service Calls\n");
            break;
        case 6:
            printf("Vendor Specific Hypervisor Service Calls\n");
        case 7 ... 47:
            printf("Reserved\n");
            break;
        case 48 ... 49:
            printf("TA Calls\n");
            break;
        case 50 ... 63:
            printf("Trusted OS Calls\n");
            break;
    }
/*
    if(call_code == QEMU_PSCI_0_2_FN_PSCI_VERSION){
        return PSCI_VERSION;
    }
*/

    return 0;

    

    


}

int main(){
    
    printf("Hello World: %lx\n", 0x32);

    start_hyp(); 
}