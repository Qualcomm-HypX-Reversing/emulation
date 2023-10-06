#include <stdint.h>
#include "printf.h"
#include "cpu_state.h"
#include "uh.h"


int main(){

    uh_call(UH_APP_INIT, 0, 0, 0, 0, 0);
    uh_call(UH_APP_RKP,1,0xdeadbeef,0xdeadb00b,0x11c001,0xfaded);
    
    while(1){
        
    }
}