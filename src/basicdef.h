#ifndef __DDFA_BASICDEF_H__
#define __DDFA_BASICDEF_H__

#include <stddef.h>
//For specifying the kind of tracing for each function call or a map.  
typedef int trace_kind_t; 

//trace each time it is called
#define TRACE_KIND_PER_CALL        0x00001
//traced once per call site location. 
#define TRACE_KIND_PER_CALLSITE    0x00010
//traced once per full call path. 
#define TRACE_KIND_PER_CALLPATH    0x00100
#define TRACE_KIND_PER_ADDR        0x01000

//Not using the constructor now
//void before_main () __attribute__((constructor));
void before_main () ;
void after_main () __attribute__((destructor));

#endif