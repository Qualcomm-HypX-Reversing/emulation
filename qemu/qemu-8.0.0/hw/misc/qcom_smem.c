/*QCOM SMEM implementation*/

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "sysemu/runstate.h"
#include "hw/misc/qcom_smem.h"
#include "exec/address-spaces.h"
#include "hw/qdev-core.h"


static void qcom_smem_realize(DeviceState *dev, Error **errp);
static void qcom_smem_class_init(ObjectClass* klass, void* data);
static void init_qcom_smem_types(void);
static uint64_t read_info_reg(void* opaque, hwaddr addr, unsigned size);





static uint64_t read_info_reg(void* opaque, hwaddr addr, unsigned size){
    
    addr += QCOM_INFO_REGION_BASE;

    switch(addr){
        case SMEM_INFO_REG_LO:
            return (uint64_t)SMEM_INFO_STRUCT_ADDR & ((1L<<32) - 1);//base
        case SMEM_INFO_REG_HI:
            return (uint64_t)SMEM_INFO_STRUCT_ADDR >> 32;
    }



    qemu_log_mask(LOG_GUEST_ERROR, "Hypervisor attempted to acccess invalid info reg address, addr: %lx\n", addr);
    return 0;
}

static const MemoryRegionOps smem_info_region_ops = {
    .read = read_info_reg,
};

static void qcom_smem_realize(DeviceState *dev, Error **errp){
    QcomSmemState* qss = QCOM_SMEM(dev);
    qss->smem_region = g_malloc0(QCOM_SMEM_REGION_SIZE);
    if(!qss->smem_region){
        qemu_log_mask(LOG_GUEST_ERROR, "Failed to create smem_region\n");
        return;
    }


    //setup memory regions
    //target reg region
    memory_region_init_io(&qss->info_region, OBJECT(qss), &smem_info_region_ops, qss,  "smem-targ-info-reg", QCOM_INFO_REGION_SIZE); 
    memory_region_add_subregion(get_system_memory(), QCOM_INFO_REGION_BASE, &qss->info_region); 

    //smem_info_header region

    Error* err = NULL;
    //smem region 
    memory_region_init_ram_from_file(&qss->smem_region_mr, OBJECT(qss), "smem-region", QCOM_SMEM_REGION_SIZE, PAGE_SIZE, 0, "bin/smem_dump", true, &err); 
    memory_region_add_subregion(get_system_memory(), QCOM_SMEM_REGION_BASE, &qss->smem_region_mr);
    
}



static void qcom_smem_class_init(ObjectClass* klass, void* data){
    DeviceClass* dc = DEVICE_CLASS(klass);
    dc->desc = QCOM_SMEM_NAME;
    dc->realize = qcom_smem_realize;

    //TODO: implement a finalize function
}



static const TypeInfo qcom_smem_info = {
    .name = QCOM_SMEM_NAME,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(QcomSmemState),
    .class_init = qcom_smem_class_init,
};


static void init_qcom_smem_types(void){
    type_register_static(&qcom_smem_info); 
}


type_init(init_qcom_smem_types);



