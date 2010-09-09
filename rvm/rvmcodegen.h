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


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
void rvm_codegen_funcstart(rvm_codegen_t *cg, ruint args);
void rvm_codegen_funcend(rvm_codegen_t *cg);

#ifdef __cplusplus
}
#endif

#endif

