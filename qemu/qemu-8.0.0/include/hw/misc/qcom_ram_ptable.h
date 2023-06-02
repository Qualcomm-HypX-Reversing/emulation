#ifndef __QCOM_RAM_PTABLE_H
#define __QCOM_RAM_PTABLE_H

//Most of these types have been taken from here: https://github.com/u-boot/u-boot/blob/cb4437e530ec1ff3deae85754010344afab8bcc5/arch/arm/mach-snapdragon/dram.c#L30

//https://android.googlesource.com/kernel/lk/+/qcom-dima-8x74-fixes/platform/msm_shared/smem.h


struct smem_ram_ptn {
	char name[16];
	unsigned start;
	unsigned size;
	/* RAM Partition attribute: READ_ONLY, READWRITE etc.  */
	unsigned attr;
	/* RAM Partition category: EBI0, EBI1, IRAM, IMEM */
	unsigned category;
	/* RAM Partition domain: APPS, MODEM, APPS & MODEM (SHARED) etc. */
	unsigned domain;
	/* RAM Partition type: system, bootloader, appsboot, apps etc. */
	unsigned type;
	/* reserved for future expansion without changing version number */
	unsigned reserved2, reserved3, reserved4, reserved5;
} __attribute__ ((__packed__));


struct smem_ram_ptable {
#define _SMEM_RAM_PTABLE_MAGIC_1 0x9DA5E0A8
#define _SMEM_RAM_PTABLE_MAGIC_2 0xAF9EC4E2
	unsigned magic[2];
	unsigned version;
	unsigned reserved1;
	unsigned len;
	struct smem_ram_ptn parts[32];
	unsigned buf;
} __attribute__ ((__packed__));



#define RAM_PTABLE_ITEM_NUMBER 0x192 //this was reversed





#endif