#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rarray.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rvm_codegen_s {
	rarray_t *code;
	ruint codeoff;

} rvm_codegen_t;


#ifdef __cplusplus
}
#endif

#endif

