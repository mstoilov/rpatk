#ifndef _RVMARRAY_H_
#define _RVMARRAY_H_

#include "rvmcpu.h"
#include "rarray.h"
#include "rharray.h"

#ifdef __cplusplus
extern "C" {
#endif

rarray_t *rvmreg_array_create();
rharray_t *rvmreg_harray_create();


#ifdef __cplusplus
}
#endif

#endif
