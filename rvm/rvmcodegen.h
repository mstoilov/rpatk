#ifndef _RVMCODEGEN_H_
#define _RVMCODEGEN_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"
#include "rvm.h"
#include "rvmcodemap.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RVM_CODEGEN_FUNCINITOFFSET 4

typedef struct rvm_codegen_s {
	rarray_t *code;
	ruint codeoff;
	rvm_codemap_t *codemap;
} rvm_codegen_t;


rvm_codegen_t *rvm_codegen_create();
void rvm_codegen_destroy(rvm_codegen_t *cg);
ruint rvm_codegen_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize, ruint args);
ruint rvm_codegen_funcstart_str(rvm_codegen_t *cg, const rchar* name, ruint args);
ruint rvm_codegen_vargs_funcstart(rvm_codegen_t *cg, const rchar* name, ruint namesize);
ruint rvm_codegen_vargs_funcstart_str(rvm_codegen_t *cg, const rchar* name);
void rvm_codegen_funcend(rvm_codegen_t *cg);
ruint rvm_codegen_addins(rvm_codegen_t *cg, rvm_asmins_t ins);
rvm_asmins_t *rvm_codegen_getcode(rvm_codegen_t *cg, ruint index);
rulong rvm_codegen_getcodesize(rvm_codegen_t *cg);

#ifdef __cplusplus
}
#endif

#endif

