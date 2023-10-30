#include <stdint.h>
#include <string.h>
#include "printf.h"
#include "cpu_state.h"
#include "smccc.h"
#include "psci.h"
#include "page_tables.h"
extern void start_hyp(uint64_t kernel_start, uint64_t should_be_2);

extern void start_kernel();

#define KERNEL_START 0x81000000

void smem_mapping_setup();

void VMMap_SLAT(unsigned long start, unsigned long size);

void *memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}


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
    static int done = 0;
    if(call_code == 0xc2000310 && !done){ //a hacky solution to trigger page table fixup at around 0x80067318 for smem. This is right before we need to access SMEM, so its the perfect time to set it up
        done = 1;
        smem_mapping_setup();
    }

    if(call_code == 0xfaded){ //0xfaded indicates that the hypervisor is ready for fuzzing
        VMMap_SLAT(KERNEL_START, 0x210000);
        VMMap_SLAT(0x09000000, 0x1000); //map uart printf for kernel
        start_kernel();

    }

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

void write_constants(){
    char* smmu_init = (char*)0x801d8ba1;//skip smmu/device initialiation as we are trying to focus on RKP
    *smmu_init = 0x1;

    /*int* memory_attr_arr_size = (int*)0x80090f7c;
    *memory_attr_arr_size = 0;*/
}


void smem_mapping_setup(){
    VMMap(0x80900000L, 0x200000); //map smem
    VMMap(0xc0000000L, 0x200000); //map page table memory
    VMMap(KERNEL_START, 0x210000); //map el1 firmware
}

void patch(){
    int* patch_addr = (int*)0x8005ac84; //patch device_memory to always return 0 thus skipping a lot of the SLAT stuff
    *patch_addr = 0x2a1f03e0;
    patch_addr = (int*)0x8005af3c; //patch the device memory enable
    *patch_addr = 0x2a1f03f4;
    patch_addr = (int*)0x800658c0;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch out map_to_HLOS
    patch_addr = (int*)0x8005af48;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch out the context bank bullshit
    patch_addr = (int*)0x80067a00;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch out the the tz diag region mapping

    patch_addr = (int*)0x8005fea8;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - fuck pmic access control

    patch_addr = (int*)0x80055ab8;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - fuck pil access control
    
    patch_addr = (int*)0x80054fec;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - no hyp shared memory bridge

    patch_addr = (int*)0x800402b0;
    
    memcpy(patch_addr, "\xa0\xbd\x95\xd2\xe0\x01\xa0\xf2\x03\x00\x00\xd4", 12);
    
}



int main(){
    write_constants();
    patch();
    start_hyp(KERNEL_START, 2); 
}