#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"
#include "rvm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RVM_CODEGEN_FUNCINITOFFSET 3

typedef struct rvm_codegen_s {
	rarray_t *code;
	ruint codeoff;
	rhash_t *funcmap;

} rvm_codegen_t;


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
void rvm_codegen_funcstart(rvm_codegen_t *cg, ruint args);
void rvm_codegen_funcend(rvm_codegen_t *cg);
void rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins);
rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg);
rulong rvm_codegen_getcodesize(rvm_codegen_t *cg);

#ifdef __cplusplus
}
#endif

#endif

