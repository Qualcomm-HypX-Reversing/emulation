/*QCOM SMEM implementation*/

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "sysemu/runstate.h"
#include "hw/misc/qcom_smem.h"
#include "exec/address-spaces.h"
#include "hw/qdev-core.h"
#include "hw/misc/qcom_ram_ptable.h"


static void qcom_smem_realize(DeviceState *dev, Error **errp);
static void qcom_smem_class_init(ObjectClass* klass, void* data);
static void init_qcom_smem_types(void);
static uint64_t read_info_reg(void* opaque, hwaddr addr, unsigned size);
static uint64_t read_smem_region(void* opaque, hwaddr addr, unsigned size);
static void* smem_alloc(int item, size_t size);
static int setup_RAM_ptable(void);


static struct smem_partition_header* global_part_ptr;



static uint64_t read_smem_region(void* opaque, hwaddr addr, unsigned size){



    struct QcomSmemState* qss = (struct QcomSmemState*)opaque;
    void* smem_region = qss->smem_region;

    switch(size){
            case 8:
                 return *(uint64_t*)(smem_region + addr);
            case 4:
                return *(uint32_t*)(smem_region + addr);
            case 2:
                return *(uint16_t*)(smem_region + addr);
            case 1: 
            return *(uint8_t*)(smem_region + addr);
        }


    qemu_log("SMEM read from offset: %lx\n", addr);
    
    return 0;
}

static void  write_smem_region(void* opaque, hwaddr addr, uint64_t data, unsigned size){
        struct QcomSmemState* qss = (struct QcomSmemState*)opaque;
    void* smem_region = qss->smem_region;

    switch(size){
            case 8:
                *(uint64_t*)(smem_region + addr) = data;
                break;
            case 4:
                *(uint32_t*)(smem_region + addr) = (uint32_t)data;
                break;
            case 2:
                *(uint16_t*)(smem_region + addr) = (uint16_t)data;
                break;
            case 1: 
                *(uint8_t*)(smem_region + addr) = (uint8_t)data;
                break;
        }


    qemu_log("SMEM write to offset: %lx\n", addr);
    
}

static const MemoryRegionOps smem_region_ops = {
    .read = read_smem_region,
    .write = write_smem_region,
};



static uint64_t read_info_reg(void* opaque, hwaddr addr, unsigned size){
    
    addr += QCOM_INFO_REGION_BASE;

    switch(addr){
        case SMEM_INFO_REG_LO:
            return (uint64_t)SMEM_INFO_STRUCT_ADDR & ((1L<<32) - 1);//base
        case SMEM_INFO_REG_HI:
            return (uint64_t)SMEM_INFO_STRUCT_ADDR >> 32;
    }
    

    if(addr >= SMEM_INFO_STRUCT_ADDR && addr < SMEM_INFO_STRUCT_ADDR + sizeof(struct smem_info)){
        switch(size){
            case 8:
                 return *(uint64_t*)(((char*)&smem_info) + addr - SMEM_INFO_STRUCT_ADDR);
            case 4:
                return *(uint32_t*)(((char*)&smem_info) + addr - SMEM_INFO_STRUCT_ADDR);
            case 2:
                return *(uint16_t*)(((char*)&smem_info) + addr - SMEM_INFO_STRUCT_ADDR);
            case 1: 
            return *(uint8_t*)(((char*)&smem_info) + addr - SMEM_INFO_STRUCT_ADDR);
        }
    }



    qemu_log_mask(LOG_GUEST_ERROR, "Hypervisor attempted to acccess invalid info reg address, addr: %lx\n", addr);
    return 0;
}

static const MemoryRegionOps smem_info_region_ops = {
    .read = read_info_reg,
};


static int setup_smem_header(struct smem_header* header){
    header -> version[SMEM_MASTER_SBL_VERSION_INDEX] = SMEM_GLOBAL_PART_VERNUM; //use the heap version because fuck that
    header -> initialized = 1; //the driver will straight panic if we don't do this.
    return 0;
}



//I am not checking for existing items. That's up to the developer.
static void* smem_alloc(int item, size_t size){
    struct smem_private_entry* next_free = (struct smem_private_entry*)((void*)global_part_ptr + global_part_ptr -> offset_free_uncached);

    __le32 region_size = (__le32)ALIGN(size,8);

    if(global_part_ptr -> offset_free_uncached + sizeof(*next_free) + region_size > global_part_ptr -> offset_free_cached)
        return NULL;

    next_free -> canary = SMEM_PRIVATE_CANARY;
    next_free -> size = region_size;
    next_free -> padding_data = (__le16)(next_free->size - size);
    next_free -> padding_hdr = 0;
    next_free -> item = item;

    global_part_ptr -> offset_free_uncached += sizeof(*next_free) + next_free -> size;

    return (void*)(++next_free);
    
}


/*
This function sets up an empty partition
*/
static void setup_partition_header(struct smem_partition_header* pheader, uint16_t host0, uint16_t host1){
    memcpy(&pheader->magic, SMEM_PART_MAGIC, sizeof(SMEM_PART_MAGIC));
    pheader->host0 = (__le16)host0;
    pheader->host1 = (__le16)host1;
    pheader->offset_free_uncached = (__le32)sizeof(struct smem_partition_header); 
    pheader->offset_free_cached = PARTITION_SIZE;
    pheader->size = PARTITION_SIZE;

    global_part_ptr = pheader;
}

static int setup_partitions(void* smem_region){

    struct smem_ptable* ptable = (struct smem_ptable*)(smem_region + QCOM_PTABLE_OFF);
    memcpy(&ptable->magic, SMEM_PTABLE_MAGIC, sizeof(SMEM_PTABLE_MAGIC));
    ptable->num_entries = SMEM_PARTITION_NUM;
    ptable->version = 1;



    struct smem_ptable_entry* ent = &ptable->entry[0]; //there is only 1 entry if we only have 1 CPU
    ent -> offset = QCOM_ALIGN_SMEM_HEADER_SIZE;
    ent -> size = PARTITION_SIZE;
    ent -> host0 = SMEM_GLOBAL_HOST;
    ent -> host1 = SMEM_GLOBAL_HOST;
    ent -> cacheline = 1; // I don't want to deal with alignment
    
    setup_partition_header((struct smem_partition_header*) (smem_region + ent->offset), SMEM_GLOBAL_HOST, SMEM_GLOBAL_HOST);

    return 0;
}


static int setup_RAM_ptable(void){
    struct smem_ram_ptable* ram_ptable = smem_alloc(RAM_PTABLE_ITEM_NUMBER, sizeof(struct smem_ram_ptable));

    if(!ram_ptable){
        qemu_log_mask(LOG_GUEST_ERROR, "Failed to allocate a RAM ptable area\n");
        return -1;
    }

    ram_ptable -> magic[0] = 0xaabbccdd; //do an invalid magic to have it skip this RAM ptable code
    ram_ptable -> magic[1] = 0x11223344;




    return 0;
}

static int setup_smem_region(void* smem_region){
    if(setup_smem_header((struct smem_header*)smem_region)){
        qemu_log_mask(LOG_GUEST_ERROR, "Failed to setup smem_header\n");
        return -1;
    }

    if(setup_partitions(smem_region)){
        qemu_log_mask(LOG_GUEST_ERROR, "Failed to setup smem_region\n");
        return -1;
    }

    if(setup_RAM_ptable()){
        return -1;
    }

    return 0;
}

static void qcom_smem_realize(DeviceState *dev, Error **errp){
    QcomSmemState* qss = QCOM_SMEM(dev);
    qss->smem_region = g_malloc0(QCOM_SMEM_REGION_SIZE);
    if(!qss->smem_region){
        qemu_log_mask(LOG_GUEST_ERROR, "Failed to create smem_region\n");
        return;
    }

    if(setup_smem_region(qss->smem_region)){
        return;
    }

    //setup memory regions
    //target reg region
    memory_region_init_io(&qss->info_region, OBJECT(qss), &smem_info_region_ops, qss,  "smem-targ-info-reg", QCOM_INFO_REGION_SIZE); 
    memory_region_add_subregion(get_system_memory(), QCOM_INFO_REGION_BASE, &qss->info_region); 

    //smem_info_header region


    //smem region 
    memory_region_init_io(&qss->smem_region_mr, OBJECT(qss), &smem_region_ops, qss, "smem-region", QCOM_SMEM_REGION_SIZE); 
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



