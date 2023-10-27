#include <stdint.h>
#include "printf.h"
#include "cpu_state.h"
#include "uh.h"
#include "rkp.h"

const extern unsigned long _text;
const extern unsigned long _etext;
const extern unsigned long _data;
const extern unsigned long _edata;
const extern unsigned long page_allocator_start;

#define EL1_LOAD 0x81000000

void *
memset (void *dest, register int val, register size_t len)
{
  register unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}


char region[4096];

int main(){

    struct rkp_init rkp_init_data = {
	.magic = RKP_MAGIC,
	.vmalloc_start = page_allocator_start,
	.no_fimc_verify = 1,
	.fimc_phys_addr = 0,
	._text = (u64)&_text,
	._etext = (u64)&_etext,
	._srodata = (u64)&_data,
	._erodata = (u64)&_edata,
	 .large_memory = 0,
	};
	

    uh_call(UH_APP_INIT, 0, 0, 0, 0, 0);
    //uh_call(UH_PREFIX | 95,0xdeadbeef,0xdeadbeef,0xdeadb00b,0x11c001,0xfaded); overflow
    uh_call(UH_APP_INIT, 2, EL1_LOAD, 0x4000000, 1, 0); //voffset and physaddr start are probably the same
  //  uh_call(UH_APP_INIT, 2, EL1_LOAD+0x1000, 0x4000000+0x1000, 1, 0); //voffset and physaddr start are probably the same


    unsigned long param_1 = (unsigned long)&region;
    unsigned long param_2 = 4096;
    uh_call(UH_APP_INIT, 4, (u64)&param_1, (u64)&param_2, 1, 0); //voffset and physaddr start are probably the same

    uh_call(UH_APP_RKP, RKP_START, (u64)&rkp_init_data, EL1_LOAD, 0, EL1_LOAD);

    
    while(1){
        
    }
}