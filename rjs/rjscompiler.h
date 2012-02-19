/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#ifndef _RJSCOMPILER_H_
#define _RJSCOMPILER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rvm/rvmcodegen.h"
#include "rvm/rvmscope.h"
#include "rvm/rvmcpu.h"
#include "rjs/rjsuids.h"
#include "rpa/rparecord.h"
#include "rjs/rjserror.h"

#define RJS_COMPILER_NHANDLERS 128
#define RJS_COMPILER_CODEGENKEEP (1 << 0)

#define RJS_COCTX_NONE 0
#define RJS_COCTX_GLOBAL (1 << 1)
#define RJS_COCTX_FUNCTION (1 << 2)
#define RJS_COCTX_FUNCTIONCALL (1 << 3)
#define RJS_COCTX_IFSTATEMENT (1 << 4)
#define RJS_COCTX_ITERATION (1 << 5)
#define RJS_COCTX_OPERATION (1 << 5)
#define RJS_COCTX_DELETE (1 << 6)
#define RJS_COCTX_SWITCH (1 << 7)


typedef struct rjs_coctx_s {
	unsigned long type;
} rjs_coctx_t;


typedef struct rjs_coctx_global_s {
	rjs_coctx_t base;
	unsigned long stackallocs;
} rjs_coctx_global_t;


typedef struct rjs_coctx_function_s {
	rjs_coctx_t base;
	long start;
	long execidx;
	long endidx;
	long allocsidx;
	long execoff;
	unsigned long stackallocs;
} rjs_coctx_function_t;


typedef struct rjs_coctx_operation_s {
	rjs_coctx_t base;
	long opcode;
} rjs_coctx_operation_t;


typedef struct rjs_coctx_functioncall_s {
	rjs_coctx_t base;
	unsigned long arguments;
	unsigned char setthis;
} rjs_coctx_functioncall_t;


typedef struct rjs_coctx_ifstatement_s {
	rjs_coctx_t base;
	long start;
	long trueidx;
	long falseidx;
	long endidx;
} rjs_coctx_ifstatement_t;


typedef struct rjs_coctx_iteration_s {
	rjs_coctx_t base;
	long start;
	long iterationidx;
	long continueidx;
	long endidx;
} rjs_coctx_iteration_t;


typedef enum rjs_compiler_pass_s {
	RJS_COMPILER_NONE = 0,
	RJS_COMPILER_PASS_1,
	RJS_COMPILER_PASS_2,
} rjs_compiler_pass_t;


typedef struct rjs_coctx_switch_s {
	rjs_coctx_t base;
	rjs_compiler_pass_t pass;
	long start;
	long endidx;
	long defaultidx;
	long casenum;
	rarray_t *caseidx;
} rjs_coctx_switch_t;


typedef struct rjs_compiler_s rjs_compiler_t;
typedef int (*RJS_COMPILER_RH)(rjs_compiler_t *co, rarray_t *records, long rec);

struct rjs_compiler_s {
	rvmcpu_t *cpu;
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	rjs_error_t *error;
	rarray_t *coctx;
	char *temp;
	rstr_t stringcharacters;
	const char *script;
	unsigned long scriptsize;
	long headoff;
	long opcode;
	unsigned long debug:1;
	RJS_COMPILER_RH handlers[RJS_COMPILER_NHANDLERS];
};


rjs_compiler_t *rjs_compiler_create(rvmcpu_t *cpu);
void rjs_compiler_destroy(rjs_compiler_t *co);
int rjs_compiler_compile(rjs_compiler_t *co, const char *script, unsigned long scriptsize, rarray_t *records, rvm_codegen_t *cg, rjs_error_t *error);
rjs_coctx_t *rjs_compiler_getctx(rjs_compiler_t *co, unsigned long type);

#ifdef __cplusplus
}
#endif

#endif
