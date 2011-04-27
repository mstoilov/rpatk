#ifndef _RJSCOMPILER_H_
#define _RJSCOMPILER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rarray.h"
#include "rvmcodegen.h"


typedef struct rjs_compiler_s {
	rvm_codegen_t *cg;
} rjs_compiler_t;


rjs_compiler_t *rjs_compiler_create();
void rjs_compiler_destroy(rjs_compiler_t *parser);


#ifdef __cplusplus
}
#endif

#endif
