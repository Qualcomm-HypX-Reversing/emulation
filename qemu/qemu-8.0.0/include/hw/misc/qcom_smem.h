#ifndef __QCOM_SMEM
#define __QCOM_SMEM

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "sysemu/runstate.h"
#include "exec/memory.h"
#include "hw/qdev-core.h"


//Various types for QCOM SMEM
//https://android.googlesource.com/kernel/msm/+/android-msm-bluecross-4.9-pie-dr1-release/arch/arm64/boot/dts/qcom/sdm845.dtsi#2151 
#define SMEM_INFO_REG_LO 0x1fd4000L //these registers give a pointer to the smem_info structure
#define SMEM_INFO_REG_HI 0x1fd4004L 
#define SMEM_INFO_STRUCT_ADDR 0x1fd4010L //16 byte align it so we don't face any weird alignment issues
#define QCOM_INFO_REGION_SIZE 0x1000L
#define QCOM_INFO_REGION_BASE SMEM_INFO_REG_LO


#ifndef ALIGN
#define ALIGN(x, y)  (((x)+(y)-1) & ~((y)-1)) 
#endif


#define QCOM_SMEM_REGION_BASE 0x1fd5000L 
#define SMEM_PARTITION_NUM 0x10
#define QCOM_SMEM_REGION_SIZE ALIGN(sizeof(struct smem_header), 0x1000) + SMEM_PARTITION_NUM*0x1000 + 0x1000 //page aligned smem_header + 1 page for each partition entries + 0x1000 for part table 
#define QCOM_PTABLE_OFF QCOM_SMEM_REGION_SIZE - 0x1000 //reversed

#define QCOM_SMEM_NAME "qcom-smem"
#define TYPE_QCOM_SMEM "qcom-smem-type"


struct QcomSmemState{
	DeviceState parent_obj;
    MemoryRegion smem_region_mr; //smem region memory region
    MemoryRegion info_region; //info reg
    void* smem_region; //
};




//A lot of this stuff has been copied from the Linux Kernel driver for Qualcomm SMEM


//types for qcom smem types
typedef uint8_t u8;
typedef uint16_t __le16;
typedef uint32_t __le32;

/**
 * struct smem_info - smem region info located after the table of contents
 * @magic:	magic number, must be SMEM_INFO_MAGIC
 * @size:	size of the smem region
 * @base_addr:	base address of the smem region
 * @reserved:	for now reserved entry
 * @num_items:	highest accepted item number
 */
struct smem_info {
	u8 magic[4];
	__le32 size;
	__le64 base_addr;
	__le64 num_items;
};

static const u8 SMEM_INFO_MAGIC[] = { 0x53, 0x49, 0x49, 0x49 }; /* SIII */





/**
 * struct smem_partition_header - header of the partitions
 * @magic:	magic number, must be SMEM_PART_MAGIC
 * @host0:	first processor/host with access to this partition
 * @host1:	second processor/host with access to this partition
 * @size:	size of the partition
 * @offset_free_uncached: offset to the first free byte of uncached memory in
 *		this partition
 * @offset_free_cached: offset to the first free byte of cached memory in this
 *		partition
 * @reserved:	for now reserved entries
 */
struct smem_partition_header {
	u8 magic[4];
	__le16 host0;
	__le16 host1;
	__le32 size;
	__le32 offset_free_uncached;
	__le32 offset_free_cached;
	__le32 reserved[3];
};

static const u8 SMEM_PART_MAGIC[] = { 0x24, 0x50, 0x52, 0x54 };


/**
 * struct smem_ptable_entry - one entry in the @smem_ptable list
 * @offset:	offset, within the main shared memory region, of the partition
 * @size:	size of the partition
 * @flags:	flags for the partition (currently unused)
 * @host0:	first processor/host with access to this partition
 * @host1:	second processor/host with access to this partition
 * @cacheline:	alignment for "cached" entries
 * @reserved:	reserved entries for later use
 */
struct smem_ptable_entry {
	__le32 offset;
	__le32 size;
	__le32 flags;
	__le16 host0;
	__le16 host1;
	__le32 cacheline;
	__le32 reserved[7];
};

/**
 * struct smem_ptable - partition table for the private partitions
 * @magic:	magic number, must be SMEM_PTABLE_MAGIC
 * @version:	version of the partition table
 * @num_entries: number of partitions in the table
 * @reserved:	for now reserved entries
 * @entry:	list of @smem_ptable_entry for the @num_entries partitions
 */
struct smem_ptable {
	u8 magic[4];
	__le32 version;
	__le32 num_entries;
	__le32 reserved[5];
	struct smem_ptable_entry entry[];
};

static const u8 SMEM_PTABLE_MAGIC[] = { 0x24, 0x54, 0x4f, 0x43 }; /* "$TOC" */

//this is located at smem_base + smem_size-0x1000







/**
  * struct smem_proc_comm - proc_comm communication struct (legacy)
  * @command:	current command to be executed
  * @status:	status of the currently requested command
  * @params:	parameters to the command
  */
struct smem_proc_comm {
	__le32 command;
	__le32 status;
	__le32 params[2];
};


/**
 * struct smem_global_entry - entry to reference smem items on the heap
 * @allocated:	boolean to indicate if this entry is used
 * @offset:	offset to the allocated space
 * @size:	size of the allocated space, 8 byte aligned
 * @aux_base:	base address for the memory region used by this unit, or 0 for
 *		the default region. bits 0,1 are reserved
 */
struct smem_global_entry {
	__le32 allocated;
	__le32 offset;
	__le32 size;
	__le32 aux_base; /* bits 1:0 reserved */
};

#define SMEM_ITEM_COUNT		512

/**
 * struct smem_header - header found in beginning of primary smem region
 * @proc_comm:		proc_comm communication interface (legacy)
 * @version:		array of versions for the various subsystems
 * @initialized:	boolean to indicate that smem is initialized
 * @free_offset:	index of the first unallocated byte in smem
 * @available:		number of bytes available for allocation
 * @reserved:		reserved field, must be 0
 * @toc:		array of references to items
 */
struct smem_header {
	struct smem_proc_comm proc_comm[4];
	__le32 version[32]; 
	__le32 initialized;
	__le32 free_offset;
	__le32 available;
	__le32 reserved;
	struct smem_global_entry toc[SMEM_ITEM_COUNT];
};


/*
version index 

if header->version[SMEM_MASTER_SBL_VERSION_INDEX] == LEGACY_ALLOCATOR then use the legacy allocator
if header->version[SMEM_MASTER_SBL_VERSION_INDEX] == MODERN_ALLOCATOR then use the modern allocator 
*/
#define SMEM_MASTER_SBL_VERSION_INDEX 7
#define LEGACY_ALLOCATOR 0xb0000
#define MODERN_ALLOCATOR  0xc0000


static struct smem_info smem_info = {
	.magic = { 0x53, 0x49, 0x49, 0x49 },
	.size = QCOM_SMEM_REGION_SIZE,
	.base_addr = QCOM_SMEM_REGION_BASE,
	.num_items = SMEM_PARTITION_NUM,
};


OBJECT_DECLARE_SIMPLE_TYPE(QcomSmemState, QCOM_SMEM)

#endif