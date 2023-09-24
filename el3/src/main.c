#include <stdint.h>
#include "printf.h"
#include "cpu_state.h"
#include "smccc.h"
#include "psci.h"
extern void start_hyp();

void smem_mapping_setup();

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
    if(call_code == 0xc2000310 && !done){ //a hacky solution to trigger page table fixup at around 0x80067318 for smem
        done = 1;
        smem_mapping_setup();
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

#define CUSTOM_PAGE_TABLE_BASE (0xc0000000L)
#define TTBR0_EL2 (0xb3800000L)

#define PAGE_SIZE (0x1000)

#define S2_SIZE (1<<12) //the number of bytes that a single stage 2 descriptor covers
#define S2_MASK (~(S2_SIZE-1))
#define S2_INDEX(addr) ((addr >> 12) & 511)


#define S1_SIZE (1<<21) //the amount of bytes that a single stage 1 descriptor covers
#define S1_MASK (~(S1_SIZE-1))
#define S1_INDEX(addr) ((addr >> 21) & 511)

#define S0_SIZE (1<<30) //the amount that a single stage 0 descriptor covers
#define S0_MASK (~(S0_SIZE-1))
#define S0_INDEX(addr) ((addr >> 30) & 511)
 
#define S0_ADDR_END(addr,end)                                  \
({                                                              \
    unsigned long __boundary = ((addr + S0_SIZE) & S0_MASK);    \
    (__boundary-1 < (end) - 1) ? __boundary : end;               \
})

#define S1_ADDR_END(addr,end)                                  \
({                                                              \
    unsigned long __boundary = ((addr + S1_SIZE) & S1_MASK);    \
    (__boundary-1 < (end) - 1) ? __boundary : end;               \
})

#define S2_ADDR_END(addr,end)                                  \
({                                                              \
    unsigned long __boundary = ((addr + S2_SIZE) & S2_MASK);    \
    (__boundary-1 < (end) - 1) ? __boundary : end;               \
})

#define GET_DESC_ADDR(addr) (addr & S2_MASK)

#define PAGE_ATTRIBUTES (0b11)
#define PTE_ATTRIBUTES (0x747)

unsigned long page_table_index = CUSTOM_PAGE_TABLE_BASE;

unsigned long read_from_phys(unsigned long addr){
    return *(unsigned long*)addr;
}

void write_to_phys(unsigned long addr, unsigned long val){
    *(unsigned long*)addr = val;
}

unsigned long get_next_page_table_index(){
    unsigned long ret = page_table_index;
    page_table_index += 4096;
    return ret;
}


void remap_s2_mapping(unsigned long addr, unsigned long end, unsigned long s1_desc_addr){ 
    unsigned long s2_table = read_from_phys(s1_desc_addr) & (~PTE_ATTRIBUTES); //desc_addr is the address of the stage 1 table
    if(!s2_table) {
        write_to_phys(s1_desc_addr, get_next_page_table_index() | PTE_ATTRIBUTES);
        s2_table = read_from_phys(s1_desc_addr) & (~PTE_ATTRIBUTES);
    }

    s2_table += (S2_INDEX(addr) * 8);
    unsigned long next = 0;
    do{
        next = S2_ADDR_END(addr, end); //S1_ADDR_END should return the virtual address of the next s1 descriptor
        write_to_phys(s2_table, addr | PTE_ATTRIBUTES);

    }while(s2_table += 8, addr = next, addr != end);

}

//addr is the address to start mapping from
//end is the address after the last to map
//s0_addr is the address of the stage 0 descriptor
void remap_s1_mapping(unsigned long addr, unsigned long end, unsigned long s0_desc_addr){ 
    unsigned long s1_table = read_from_phys(s0_desc_addr) & (~PAGE_ATTRIBUTES); //desc_addr is the address of the stage 1 table
    if(!s1_table) {
        write_to_phys(s0_desc_addr, get_next_page_table_index() | PAGE_ATTRIBUTES);
        s1_table = read_from_phys(s0_desc_addr) & (~PAGE_ATTRIBUTES);
    }


    s1_table += (S1_INDEX(addr) * 8);
    unsigned long next = 0;
    do{

        next = S1_ADDR_END(addr, end); //S1_ADDR_END should return the virtual address of the next s1 descriptor
        remap_s2_mapping(addr, next, s1_table);
    }while(s1_table += 8, addr =  next, addr != end);

}

//the hypervisor uses 36 bit page tables
//basically a blatant copy of remap_pfn_range/ioremap
void VMMap(unsigned long addr, unsigned long size){ //create page table mappings for a region
    unsigned long end = addr + size;
    unsigned long s0_table = TTBR0_EL2;
    s0_table += (S0_INDEX(addr) * 8);
    unsigned long next = 0;
    do {
        next = S0_ADDR_END(addr, end); //S0_ADDR_END should return the virtual address of the next s0 desc we need to map. If the final address is in this desc, then return that as the end will be at a lower granualarity
        remap_s1_mapping(addr, next, s0_table);
    }while(s0_table+=8, addr = next, addr != end);
}



void smem_mapping_setup(){
    VMMap(0x80900000L, 0x200000);
    VMMap(0xc0000000L, 0x200000); 
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
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch fuck pmic access control

    patch_addr = (int*)0x80055ab8;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch fuck pil access control
    
    patch_addr = (int*)0x80054fec;
    *(long*)patch_addr = 0xd65f03c0aa1f03e0; //mov x0, xzr; ret - patch no hyp shared memory bridge
}



int main(){
    write_constants();
    patch();
    start_hyp(); 
}