#ifndef _RJS_H_
#define _RJS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include "rtypes.h"
#include "rvm/rvmcpu.h"
#include "rjs/rjsparser.h"
#include "rjs/rjscompiler.h"
#include "rjs/rjserror.h"


typedef struct rjs_engine_s {
	rjs_parser_t *pa;
	rjs_compiler_t *co;
	rarray_t *cgs;
	rarray_t *errors;
	rvmcpu_t *cpu;
	rlong debugcompile:1;
	rlong debugexec:1;
} rjs_engine_t;


#define RJS_VERSION_MAJOR 0
#define RJS_VERSION_MINOR 51
#define RJS_VERSION_MICRO 1
#define RJS_VERSION_STRING "0.51.1"
#define RJS_SWI_PARAM(__cpu__, __n__) RVM_STACK_ADDR((__cpu__)->stack, RVM_CPUREG_GETU(__cpu__, FP) + (__n__))
#define RJS_SWI_PARAMS(__cpu__) (RVM_CPUREG_GETU((__cpu__), SP) - RVM_CPUREG_GETU((__cpu__), FP))
#define RJS_SWI_ABORT(__j__, __e__) do { rjs_engine_abort((__j__), (__e__)); return; } while (0)


const rchar *rjs_version();

rjs_engine_t *rjs_engine_create();
void rjs_engine_destroy(rjs_engine_t *jse);
rinteger rjs_engine_open(rjs_engine_t *jse);
rinteger rjs_engine_compile(rjs_engine_t *jse, const rchar *script, rsize_t size);
rinteger rjs_engine_compile_s(rjs_engine_t *jse, const rchar *script);
rinteger rjs_engine_close(rjs_engine_t *jse);
rinteger rjs_engine_run(rjs_engine_t *jse);
rinteger rjs_engine_addswitable(rjs_engine_t *jse, const rchar *tabname, rvm_switable_t *switalbe);
rinteger rjs_engine_dumpast(rjs_engine_t *jse, const rchar *script, rsize_t size);
rvmreg_t *rjs_engine_exec(rjs_engine_t *jse, const rchar *script, rsize_t size);
rvmreg_t *rjs_engine_vexec(rjs_engine_t *jse, const rchar *script, rsize_t size, rsize_t nargs, va_list args);
rvmreg_t *rjs_engine_args_exec(rjs_engine_t *jse, const rchar *script, rsize_t size, rsize_t nargs, ...);
rvmreg_t *rjs_engine_args_exec_s(rjs_engine_t *jse, const rchar *script, rsize_t nargs, ...);
rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const rchar *script);
void rjs_engine_abort(rjs_engine_t *jse, rjs_error_t *error);
rjs_engine_t *rjs_engine_get(rvmcpu_t *cpu);

#ifdef __cplusplus
}
#endif

#endif
