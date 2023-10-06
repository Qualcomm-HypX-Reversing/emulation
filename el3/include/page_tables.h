#ifndef __PAGE_TABLE_H
#define __PAGE_TABLE_H

#define CUSTOM_PAGE_TABLE_BASE (0xc0000000L)
#define TTBR0_EL2 (0xb3800000L)
#define VTTBR_EL2 (0xb3808000L)

#define PAGE_SIZE (0x1000) //both SLAT and S1 use 4 KiB gran

#define PA_SIZE (36) //the PA size is 36 bit for both SLAT and S1

#define DESC_MASK (((1L << PA_SIZE) - 1) & ~(PAGE_SIZE - 1)) 

#define DESC_TO_TABLE(desc) (desc & DESC_MASK)

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

#define EL2_DESC_ATTRIBUTES (0b11)
#define EL2_PTE_ATTRIBUTES (0x747)

#define SLAT_PTE_ATTRIBUTES (0x4ff)
#define SLAT_DESC_ATTRIBUTES (0b00000011)


void remap_s2_mapping(unsigned long addr, unsigned long end, unsigned long s1_desc_addr, unsigned long desc_attributes, unsigned long pte_attributes);

void remap_s1_mapping(unsigned long addr, unsigned long end, unsigned long s0_desc_addr, unsigned long desc_attributes, unsigned long pte_attributes);

void VMMap(unsigned long addr, unsigned long size);

void VMMap_SLAT(unsigned long addr, unsigned long size);

#endif