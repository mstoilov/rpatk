#ifndef _RPACOMPILER_H_
#define _RPACOMPILER_H_

#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rpavm.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
} rpa_compiler_t;


rpa_compiler_t *rpa_compiler_create();
void rpa_compiler_destroy(rpa_compiler_t *co);


#ifdef __cplusplus
}
#endif

#endif
