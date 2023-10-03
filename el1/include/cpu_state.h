#ifndef CPU_STATE_H
#define CPU_STATE_H

#include <stdint.h>

#define GP_REG_NUM 31

struct  cpu_state {
    uint64_t x[GP_REG_NUM];
    uint64_t elr_el3;
    uint64_t spsr_el3;
    uint64_t esr_el3;
};

#endif