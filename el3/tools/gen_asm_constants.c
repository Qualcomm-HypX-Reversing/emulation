#include <stdio.h>
#include "cpu_state.h"

int main(void){

    FILE* fp = fopen("include/assembly-constants.h", "w+");
    if(fp == NULL){
        perror("Failed to open assembly-constants.h");
        return -1;
    }
    fprintf(fp, "/* auto-generated by gen-asm-constants.c*/\n"); 
    fprintf(fp, ".equ CPU_STATE_SIZE, %ld\n", sizeof(struct cpu_state));

    fclose(fp);
}