#ifndef _RJS_H_
#define _RJS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rvmcpu.h"
#include "rjsparser.h"
#include "rjscompiler.h"

typedef struct rjs_engine_s {
	rjs_parser_t *pa;
	rjs_compiler_t *co;
	rarray_t *cgs;
	rvmcpu_t *cpu;
	rlong debugcompile:1;
	rlong debugexec:1;
} rjs_engine_t;


#define RJS_VERSION_MAJOR 0
#define RJS_VERSION_MINOR 51
#define RJS_VERSION_MICRO 1
#define RJS_VERSION_STRING "0.51.1"

const rchar *rjs_version();

rjs_engine_t *rjs_engine_create();
void rjs_engine_destroy(rjs_engine_t *jse);
rint rjs_engine_open(rjs_engine_t *jse);
rint rjs_engine_compile(rjs_engine_t *jse, const rchar *script, rsize_t size);
rint rjs_engine_compile_s(rjs_engine_t *jse, const rchar *script);
rint rjs_engine_close(rjs_engine_t *jse);
rint rjs_engine_run(rjs_engine_t *jse);
rint rjs_engine_dumpast(rjs_engine_t *jse, const rchar *script, rsize_t size);
rvmreg_t *rjs_engine_exec(rjs_engine_t *jse, const rchar *script, rsize_t size);
rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const rchar *script);


#ifdef __cplusplus
}
#endif

#endif
