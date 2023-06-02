#include <stdint.h>
#include "printf.h"

extern void goto_hyp();

int main(){
    
    printf("Hello World: %lx\n", 0x32);

    goto_hyp(); 
}