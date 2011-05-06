#ifndef _RJSCOMPILER_H_
#define _RJSCOMPILER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rarray.h"
#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rvmcpu.h"
#include "rjsuids.h"
#include "rparecord.h"

#define RJS_COMPILER_NHANDLERS 128

#define RJS_COCTX_NONE 0
#define RJS_COCTX_GLOBAL (1 << 1)
#define RJS_COCTX_FUNCTION (1 << 2)
#define RJS_COCTX_FUNCTIONCALL (1 << 3)
#define RJS_COCTX_IFSTATEMENT (1 << 4)

typedef struct rjs_coctx_s {
	rulong type;
} rjs_coctx_t;


typedef struct rjs_coctx_global_s {
	rjs_coctx_t base;
	rsize_t allocs;
} rjs_coctx_global_t;


typedef struct rjs_coctx_function_s {
	rjs_coctx_t base;
	rsize_t allocs;
} rjs_coctx_function_t;


typedef struct rjs_coctx_functioncall_s {
	rjs_coctx_t base;
	rsize_t arguments;
} rjs_coctx_functioncall_t;


typedef struct rjs_coctx_ifstatement_s {
	rjs_coctx_t base;
	rlong start;
	rlong trueidx;
	rlong falseidx;
	rlong endidx;
} rjs_coctx_ifstatement_t;


typedef struct rjs_compiler_s rjs_compiler_t;
typedef rint (*RJS_COMPILER_RH)(rjs_compiler_t *co, rarray_t *records, rlong rec);

struct rjs_compiler_s {
	rvmcpu_t *cpu;
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	rarray_t *coctx;
	rchar *temp;
	rlong headoff;
	rlong opcode;
	rulong debug:1;
	RJS_COMPILER_RH handlers[RJS_COMPILER_NHANDLERS];
};


rjs_compiler_t *rjs_compiler_create(rvmcpu_t *cpu);
void rjs_compiler_destroy(rjs_compiler_t *co);
rint rjs_compiler_compile(rjs_compiler_t *co, rarray_t *records);
rjs_coctx_t *rjs_compiler_getctx(rjs_compiler_t *co, rulong type);

#ifdef __cplusplus
}
#endif

#endif
