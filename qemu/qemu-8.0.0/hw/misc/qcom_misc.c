//creating mappings for random small pages. THese have all been pulled off the phone

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "sysemu/runstate.h"
#include "exec/address-spaces.h"
#include "hw/qdev-core.h"
#include "hw/misc/qcom_misc.h"
#include "qapi/error.h"



void dummy_write(void* opaque, hwaddr addr, uint64_t data, unsigned size){
    if(opaque)
        qemu_log("Writing to %s region - addr: %lx\n", (char*)opaque, addr);
    else   
        qemu_log("Writing to region via dummy_write - addr: %lx\n", addr);
}

//DAL Magic

static const MemoryRegionOps dal_magic_ops = {
    .read = read_magic_region,
    .write = dummy_write,
};


uint64_t read_magic_region(void* opaque, hwaddr addr, unsigned size){

    if(size != 4){
        qemu_log("magic region was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }

    if(addr == 0){
        return 0x60080201; 
    } else {
        return 0;
    }

}

#define DAL_MAGIC_BASE 0x1fc8000
#define DAL_MAGIC_SIZE 0x1000
void initialize_magic_region(QcomMiscState* qcom_misc){
    memory_region_init_io(&qcom_misc->dal_magic_region, OBJECT(qcom_misc), &dal_magic_ops, (void*)"dal_magic_region", "dal_magic_region", DAL_MAGIC_SIZE);
    memory_region_add_subregion(get_system_memory(), DAL_MAGIC_BASE, &qcom_misc->dal_magic_region); 
}



//GLB_CTX_REGION

#define GLB_CTX_REGION_BASE 0x1f49000
#define GLB_CTX_REGION_SIZE 0x1000
void initialize_glb_mutex_region(QcomMiscState* qcom_misc){
    Error* e = NULL;


    memory_region_init_ram(&qcom_misc->glb_ctx_mutex_region, NULL, "glb_ctx_region", GLB_CTX_REGION_SIZE, &e);
    
    if(e != NULL){
        qemu_log("Error in creating glb_ctx_region\n");
        return;
    }

    memory_region_add_subregion(get_system_memory(), GLB_CTX_REGION_BASE, &qcom_misc->glb_ctx_mutex_region);

    
}   


//CLOCK_REGION

#define CLOCK_REGION_BASE 0x17c000
#define CLOCK_REGION_SIZE 0x1000
#define GPU_CLOCK_REGION_BASE 0x3D92000

static const MemoryRegionOps clock_region_ops = {
    .read = read_clock_region,
    .write = dummy_write,
};

uint64_t read_clock_region(void* opaque, hwaddr addr, unsigned size){
    
    if(size != 4){
        qemu_log("Clock region was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }

    switch(addr){
        case 0x0:
            return 0x80000000;
        case 0x14 ... 0x30:
            return 0x80000002; //original value was 0x80000000
        case 0x3c:
            return 0x1;
        case 0x50 ... 0x6c:
            return 0x1;
        default:
            return 0;
    }
}



void init_clock_region(QcomMiscState* qcom_misc){
    memory_region_init_io(&qcom_misc->clock_region, OBJECT(qcom_misc), &clock_region_ops, (void*)"clock_region", "clock_region", CLOCK_REGION_SIZE);

    
    
    memory_region_add_subregion(get_system_memory(), CLOCK_REGION_BASE, &qcom_misc->clock_region);

}


//GPU CLOCK_REGION

#define GPU_CLOCK_REGION_SIZE 0x1000
#define GPU_CLOCK_REGION_BASE 0x3D92000

static const MemoryRegionOps gpu_clock_region_ops = {
    .read = read_gpu_clock_region,
    .write = dummy_write,
};

uint64_t read_gpu_clock_region(void* opaque, hwaddr addr, unsigned size){
    
    if(size != 4){
        qemu_log("GPU clock region was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }

    switch(addr){
        case 0x0:
            return 0x80000000;
        case 0x4:
            return (uint32_t)-1;
        default:
            return 0x0;
    }
}



void init_gpu_clock_region(QcomMiscState* qcom_misc){
    memory_region_init_io(&qcom_misc->gpu_clock_region, OBJECT(qcom_misc), &gpu_clock_region_ops, (void*)"gpu_clock_region", "gpu_clock_region", GPU_CLOCK_REGION_SIZE);

    
    
    memory_region_add_subregion(get_system_memory(), GPU_CLOCK_REGION_BASE, &qcom_misc->gpu_clock_region);

}


//ARM CLOCK

//https://developer.arm.com/documentation/ddi0406/c/Appendices/System-level-implementation-of-the-Generic-Timer/The-CNTBaseN-and-CNTPL0BaseN-frames - ARM mem clock documentation

#define ARM_CLOCK_BASE 0x17c21000
#define ARM_CLOCK_REGION_SIZE 0x1000

static const MemoryRegionOps arm_clock_ops = {
    .read = read_arm_clock,
    .write = dummy_write,
};


uint64_t read_arm_clock(void* opaque, hwaddr addr, unsigned size){
    static int counter = 0xc278c92f;
    if(size != 4){
        qemu_log("ARM clock was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }
    switch(addr){
        case 0x0:
            return counter++;
        case 0x4:
            return 0x5;
        case 0x8:
            return 0xc278ca4e;
        case 0xc:
            return 0x5;
        case 0x10:
            return 0x124f800;
        case 0x28:
            return 0x3d8732cc;
        case 0x38:
            return 0x3d873160;
        case 0xfd0:
            return 0x20020000;
        default:
            return 0;
    }
}

void init_arm_clock_region(QcomMiscState* qcom_misc){
    memory_region_init_io(&qcom_misc->arm_clock_region, OBJECT(qcom_misc), &arm_clock_ops, (void*)"arm_clock", "arm_clock", ARM_CLOCK_REGION_SIZE);

    memory_region_add_subregion(get_system_memory(), ARM_CLOCK_BASE, &qcom_misc->arm_clock_region);
}


//some random aperture function

#define APERTURE_REGION_BASE 0x784218
#define APERTURE_REGION_SIZE 0x4

static const MemoryRegionOps aperture_ops = {
    .read = read_aperture_region,
    .write = dummy_write,
};


uint64_t read_aperture_region(void* opaque, hwaddr addr, unsigned size){
    
    if(size != 4){
        qemu_log("aperture region was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }
    return 0;
}

void init_aperture_region(QcomMiscState* qcom_misc){
    memory_region_init_io(&qcom_misc->aperture_region, OBJECT(qcom_misc), &aperture_ops, (void*)"aperture", "aperture", APERTURE_REGION_SIZE);

    memory_region_add_subregion(get_system_memory(), APERTURE_REGION_BASE,&qcom_misc->aperture_region);
}

//CMD_DB


#define CMD_DB_REG_BASE 0x0c3f000c
#define CMD_DB_REG_SIZE 0x4

#define CMD_DB_BASE 0x80860000
#define CMD_DB_SIZE 0x1000

static const MemoryRegionOps cmd_db_reg_ops = {
    .read = read_cmd_db_reg,
    .write = dummy_write,
};


uint64_t read_cmd_db_reg(void* opaque, hwaddr addr, unsigned size){
    
    if(size != 4){
        qemu_log("cmd_db_reg region was accessed in a size other than 4 bytes: 0x%x, offset: 0x%lx\n", size, addr);
    }
    return CMD_DB_BASE;
}

void init_cmd_db_reg(QcomMiscState* qcom_misc){
    Error* err = NULL;
    memory_region_init_io(&qcom_misc->cmd_db_reg_region, OBJECT(qcom_misc), &cmd_db_reg_ops, (void*)"cmd_db_reg", "cmd_db_reg", CMD_DB_REG_SIZE);

    memory_region_add_subregion(get_system_memory(), CMD_DB_REG_BASE,&qcom_misc->cmd_db_reg_region);
    memory_region_init_ram_from_file(&qcom_misc->cmd_db_region, OBJECT(qcom_misc), "cmd-db-region", CMD_DB_BASE, CMD_DB_SIZE, 0, "bin/cmd_db_dump", true, &err); 
    memory_region_add_subregion(get_system_memory(), 0x80860000,&qcom_misc->cmd_db_region);
    
}


//REALIZE FUNCTIONS


void qcom_misc_realize(DeviceState* dev, Error** errp){
    QcomMiscState* qcom_misc = QCOM_MISC(dev);

    initialize_magic_region(qcom_misc);
    initialize_glb_mutex_region(qcom_misc);
    init_clock_region(qcom_misc);
    init_arm_clock_region(qcom_misc);
    init_gpu_clock_region(qcom_misc);
    init_aperture_region(qcom_misc);
    init_cmd_db_reg(qcom_misc);
    
}


static void qcom_misc_class_init(ObjectClass* klass, void* data){
    DeviceClass* dc = DEVICE_CLASS(klass);
    dc->desc = QCOM_MISC_NAME;
    dc->realize = qcom_misc_realize;

    //TODO: implement a finalize function
}


static const TypeInfo qcom_misc_info = {
    .name = QCOM_MISC_NAME,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(QcomMiscState),
    .class_init = qcom_misc_class_init,
};

static void init_qcom_misc_types(void){
    type_register_static(&qcom_misc_info); 
}


type_init(init_qcom_misc_types);