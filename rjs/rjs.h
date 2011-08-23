/*
 *  Regular Pattern Analyzer (RPA)
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
	rmap_t *props[RVM_DTYPE_SIZE];
	rvmcpu_t *cpu;
	long debugcompile:1;
	long debugexec:1;
} rjs_engine_t;


#define RJS_VERSION_MAJOR 0
#define RJS_VERSION_MINOR 51
#define RJS_VERSION_MICRO 1
#define RJS_VERSION_STRING "0.51.1"
#define RJS_SWI_PARAM(__cpu__, __n__) RVM_STACK_ADDR((__cpu__)->stack, RVM_CPUREG_GETU(__cpu__, FP) + (__n__))
#define RJS_SWI_PARAMS(__cpu__) (RVM_CPUREG_GETU((__cpu__), SP) - RVM_CPUREG_GETU((__cpu__), FP))
#define RJS_SWI_ABORT(__j__, __e__) do { rjs_engine_abort((__j__), (__e__)); return; } while (0)

#define RJS_CAST			RVM_USER130		/* Cast: op1 = (op3)op2 */
#define RJS_EMOV			RVM_USER131
#define RJS_ENEG			RVM_USER132		/* Negative: op1 = -op2, Update the status register */
#define RJS_EADD			RVM_USER133		/* Add: op1 = op2 + op3, update the status register */
#define RJS_ESUB			RVM_USER134		/* Subtract: op1 = op2 - op3, update the status register */
#define RJS_EMUL			RVM_USER135		/* Multiply: op1 = op2 * op3, update the status register */
#define RJS_EDIV			RVM_USER136		/* Divide: op1 = op2 / op3, update the status register */
#define RJS_EMOD			RVM_USER137		/* Modulo: op1 = op2 % op3, update the status register */
#define RJS_ELSL			RVM_USER138		/* Logical Shift Left: op1 = op2 << op3, update the status register */
#define RJS_ELSR			RVM_USER139		/* Logical Shift Right: op1 = op2 >> op3, update the status register */
#define RJS_ELSRU			RVM_USER140		/* Logical Unsigned Shift Right: op1 = op2 >>> op3, update the status register */
#define RJS_EAND			RVM_USER141		/* Bitwise AND: op1 = op2 & op3, update status register */
#define RJS_EORR			RVM_USER142		/* Bitwise OR: op1 = op2 | op3, update the status register */
#define RJS_EXOR			RVM_USER143		/* Bitwise XOR: op1 = op2 ^ op3, update the status register */
#define RJS_ENOT			RVM_USER144		/* Bitwise NOT: op1 = ~op2, Update the status register */
#define RJS_ELAND			RVM_USER145		/* Logical AND: op1 = op2 && op3, update status register */
#define RJS_ELOR			RVM_USER146		/* Logical OR: op1 = op2 || op3, update the status register */
#define RJS_ELNOT			RVM_USER147		/* Logical NOT: op1 = !op2, update the status register */
#define RJS_EEQ				RVM_USER148		/* op1 = op2 == op3 ? 1 : 0, update the status register */
#define RJS_ENOTEQ			RVM_USER149		/* op1 = op2 != op3 ? 1 : 0, update the status register */
#define RJS_EGREAT			RVM_USER150		/* op1 = op2 > op3 ? 1 : 0, update the status register */
#define RJS_EGREATEQ		RVM_USER151		/* op1 = op2 >= op3 ? 1 : 0, update the status register */
#define RJS_ELESS			RVM_USER152		/* op1 = op2 < op3 ? 1 : 0, update the status register */
#define RJS_ELESSEQ			RVM_USER153		/* op1 = op2 <= op3 ? 1 : 0, update the status register */
#define RJS_ECMP			RVM_USER154		/* Compare: status register is updated based on the result: op1 - op2 */
#define RJS_ECMN			RVM_USER155		/* Compare Negative: status register is updated based on the result: op1 + op2 */
#define RJS_PROPLKUP		RVM_USER156		/* Lookup property */
#define RJS_PROPLKUPADD		RVM_USER157		/* Lookup or add property */
#define RJS_PROPLDR			RVM_USER158		/* Load property */
#define RJS_PROPSTR			RVM_USER159		/* Store property */
#define RJS_PROPADDR		RVM_USER160		/* Property Address */
#define RJS_PROPKEYLDR		RVM_USER161		/* Load Property Key */
#define RJS_PROPDEL			RVM_USER162		/* Delete Property */
#define RJS_PROPNEXT		RVM_USER163
#define RJS_PROPPREV		RVM_USER164
#define RJS_STRALLOC		RVM_USER165		/* Allocate string in op1, op2 is pointer (char*) to string, op3 is the size */
#define RJS_ARRALLOC		RVM_USER166		/* Allocate array in op1, op2 is the size */
#define RJS_MAPALLOC		RVM_USER167		/* Allocate array in op1, op2 is the size */

#define RJS_GPKEY_NONE 0
#define RJS_GPKEY_TYPES 1
#define RJS_GPKEY_PROPS 2

const char *rjs_version();

rjs_engine_t *rjs_engine_create();
void rjs_engine_destroy(rjs_engine_t *jse);
int rjs_engine_open(rjs_engine_t *jse);
int rjs_engine_compile(rjs_engine_t *jse, const char *script, unsigned long size);
int rjs_engine_compile_s(rjs_engine_t *jse, const char *script);
int rjs_engine_close(rjs_engine_t *jse);
int rjs_engine_run(rjs_engine_t *jse);
int rjs_engine_addswitable(rjs_engine_t *jse, const char *tabname, rvm_switable_t *switalbe);
int rjs_engine_dumpast(rjs_engine_t *jse, const char *script, unsigned long size);
rvmreg_t *rjs_engine_exec(rjs_engine_t *jse, const char *script, unsigned long size);
rvmreg_t *rjs_engine_vexec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, va_list args);
rvmreg_t *rjs_engine_args_exec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, ...);
rvmreg_t *rjs_engine_args_exec_s(rjs_engine_t *jse, const char *script, unsigned long nargs, ...);
rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const char *script);
void rjs_engine_abort(rjs_engine_t *jse, rjs_error_t *error);
rjs_engine_t *rjs_engine_get(rvmcpu_t *cpu);

#ifdef __cplusplus
}
#endif

#endif
