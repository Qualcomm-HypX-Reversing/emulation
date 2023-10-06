#include "page_tables.h"

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


void remap_s2_mapping(unsigned long addr, unsigned long end, unsigned long s1_desc_addr, unsigned long desc_attributes, unsigned long pte_attributes){ 
    unsigned long s2_table = DESC_TO_TABLE(read_from_phys(s1_desc_addr)); //desc_addr is the address of the stage 1 table
    if(!s2_table) {
        write_to_phys(s1_desc_addr, get_next_page_table_index() | desc_attributes);
        s2_table = read_from_phys(s1_desc_addr) & (~desc_attributes);
    }

    s2_table += (S2_INDEX(addr) * 8);
    unsigned long next = 0;
    do{
        next = S2_ADDR_END(addr, end); //S2_ADDR_END should return the virtual address of the next s2 descriptor if we need to map it
        write_to_phys(s2_table, addr | pte_attributes);

    }while(s2_table += 8, addr = next, addr != end);

}

//addr is the address to start mapping from
//end is the address after the last to map
//s0_addr is the address of the stage 0 descriptor
void remap_s1_mapping(unsigned long addr, unsigned long end, unsigned long s0_desc_addr, unsigned long desc_attributes, unsigned long pte_attributes){ 
    unsigned long s1_table = DESC_TO_TABLE(read_from_phys(s0_desc_addr)); //desc_addr is the address of the stage 1 table
    if(!s1_table) {
        write_to_phys(s0_desc_addr, get_next_page_table_index() | desc_attributes);
        s1_table = read_from_phys(s0_desc_addr) & (~desc_attributes);
    }


    s1_table += (S1_INDEX(addr) * 8);
    unsigned long next = 0;
    do{

        next = S1_ADDR_END(addr, end); //S1_ADDR_END should return the virtual address of the next s1 descriptor if we need to map it
        remap_s2_mapping(addr, next, s1_table, desc_attributes, pte_attributes);
    }while(s1_table += 8, addr =  next, addr != end);

}

//the hypervisor uses 36 bit page tables
//basically a blatant copy of remap_pfn_range/ioremap
void VMMap(unsigned long addr, unsigned long size){ //create page table mappings for a region
    unsigned long end = addr + size;
    unsigned long s0_table = TTBR0_EL2;
    s0_table += (S0_INDEX(addr) * 8); //address of the s0 desc
    unsigned long next = 0;
    do {
        next = S0_ADDR_END(addr, end); //S0_ADDR_END should return the virtual address of the next s0 desc if we need to map. If the final address is in this desc, then return that as the end will be at a lower granualarity
        remap_s1_mapping(addr, next, s0_table, EL2_DESC_ATTRIBUTES, EL2_PTE_ATTRIBUTES);
    }while(s0_table+=8, addr = next, addr != end);
}




void VMMap_SLAT(unsigned long addr, unsigned long size){
    unsigned long end = addr + size;
    unsigned long s0_table = VTTBR_EL2;
    s0_table += (S0_INDEX(addr) * 8);
    unsigned long next = 0;
    do {
        next = S0_ADDR_END(addr, end); //S0_ADDR_END should return the virtual address of the next s0 desc we need to map. If the final address is in this desc, then return that as the end will be at a lower granualarity
        remap_s1_mapping(addr, next, s0_table, SLAT_DESC_ATTRIBUTES, SLAT_PTE_ATTRIBUTES);
    }while(s0_table+=8, addr = next, addr != end);
}
