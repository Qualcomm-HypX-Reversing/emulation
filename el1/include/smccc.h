#ifndef SMCCC_H
#define SMCCC_H

#define IS_64(x) ((x >> 30) & 1)
#define SERVICE_CALL_NUMBER(x) ((x >> 24) & ((1<<6) - 1))
#define FUNCTION_NUMBER(x) (x & 0xffff) 
#define IS_FAST_CALL(x) ((x >> 31) & 1)



#endif