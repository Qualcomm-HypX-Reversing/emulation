#ifndef __QCOM_MISC
#define __QCOM_MISC

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "sysemu/runstate.h"
#include "exec/memory.h"
#include "hw/qdev-core.h"


#define QCOM_MISC_NAME "qcom-misc"
#define TYPE_QCOM_MISC "qcom-misc-type"



struct QcomMiscState {
    DeviceState parent_obj;
    MemoryRegion dal_magic_region;
    MemoryRegion glb_ctx_mutex_region;
    MemoryRegion clock_region;
    MemoryRegion arm_clock_region;
    MemoryRegion gpu_clock_region;
    MemoryRegion aperture_region;
    MemoryRegion cmd_db_reg_region;
    MemoryRegion cmd_db_region;
};



OBJECT_DECLARE_SIMPLE_TYPE(QcomMiscState, QCOM_MISC)


void qcom_misc_realize(DeviceState* dev, Error** errp);

void initialize_magic_region(QcomMiscState* qcom_misc);
uint64_t read_magic_region(void* opaque, hwaddr addr, unsigned size);

void initialize_glb_mutex_region(QcomMiscState* qcom_misc);

void init_clock_region(QcomMiscState* qcom_misc);
uint64_t read_clock_region(void* opaque, hwaddr addr, unsigned size);

void write_clock_region(void* opaque, hwaddr addr, uint64_t data, unsigned size);
void dummy_write(void* opaque, hwaddr addr, uint64_t data, unsigned size);

uint64_t read_arm_clock(void* opaque, hwaddr addr, unsigned size);
void init_arm_clock_region(QcomMiscState* qcom_misc);

void init_gpu_clock_region(QcomMiscState* qcom_misc);
uint64_t read_gpu_clock_region(void* opaque, hwaddr addr, unsigned size);

uint64_t read_aperture_region(void* opaque, hwaddr addr, unsigned size);
void init_aperture_region(QcomMiscState* qcom_misc);

uint64_t read_cmd_db_reg(void* opaque, hwaddr addr, unsigned size);
void init_cmd_db_reg(QcomMiscState* qcom_misc);

#endif
