#ifndef __DDFA_BASICDEF_H__
#define __DDFA_BASICDEF_H__

#include <stddef.h>
/**
 * For specifying the kind of tracing for each function call or a map.  
 */
typedef enum trace_kind {
    TRACE_KIND_PER_CALL,  //traced each time it is called 
    TRACE_KIND_PER_LOCATION, //traced once per location. Location is defined as either call_site (default) or a call_path.
    TRACE_KIND_PER_ADDR, 
} trace_kind_t;


//Not using the constructor now
//void before_main () __attribute__((constructor));
void before_main () ;
void after_main () __attribute__((destructor));

#endif