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

#include <stdarg.h>
#include "rlib/rmem.h"
#include "rlib/rmap.h"
#include "rjs/rjs.h"
#include "rvm/rvmcodegen.h"

static void rjs_engine_initgp(rjs_engine_t *jse);
static void rjs_engine_print(rvmcpu_t *cpu, rvm_asmins_t *ins);
static void rjs_engine_dbgprint(rvmcpu_t *cpu, rvm_asmins_t *ins);
static void rjs_engine_object(rvmcpu_t *cpu, rvm_asmins_t *ins);

static rvm_switable_t rjsswitable[] = {
		{"print", rjs_engine_print},
		{"dbgprint", rjs_engine_dbgprint},
		{"Object", rjs_engine_object},
		{"Array", rjs_engine_object},
		{NULL, NULL},
};


const char *rjs_version()
{
	return RJS_VERSION_STRING;
}


rjs_engine_t *rjs_engine_create()
{
	rvmreg_t *tp;
	rjs_engine_t *jse = (rjs_engine_t *) r_zmalloc(sizeof(*jse));

	jse->pa = rjs_parser_create();
	jse->cpu = rvm_cpu_create_default();
	jse->co = rjs_compiler_create(jse->cpu);
	jse->cgs = r_array_create(sizeof(rvm_codegen_t*));
	jse->errors = r_array_create(sizeof(rjs_error_t));
	jse->cpu->userdata1 = jse;
	rvm_cpu_addswitable(jse->cpu, "rjsswitable", rjsswitable);
	if (!jse->pa || !jse->cpu || !jse->co || !jse->cgs)
		goto error;
	rjs_engine_initgp(jse);
	tp = rvm_cpu_alloc_global(jse->cpu);
	rvm_reg_setjsobject(tp, (robject_t *)r_map_create(sizeof(rvmreg_t), 7));
	r_gc_add(jse->cpu->gc, (robject_t*)RVM_REG_GETP(tp));
	rvm_cpu_setreg(jse->cpu, TP, tp);
	return jse;
error:
	rjs_engine_destroy(jse);
	return NULL;
}


void rjs_engine_destroy(rjs_engine_t *jse)
{
	unsigned long i;

	if (jse) {
		if (jse->cgs) {
			for (i = 0; i < r_array_length(jse->cgs); i++) {
				rvm_codegen_destroy(r_array_index(jse->cgs, i, rvm_codegen_t*));
			}
		}
		r_array_destroy(jse->cgs);
		r_array_destroy(jse->errors);
		rjs_parser_destroy(jse->pa);
		rvm_cpu_destroy(jse->cpu);
		rjs_compiler_destroy(jse->co);
		r_free(jse);
	}
}


static void rjs_engine_addtypename(rjs_engine_t *jse, rmap_t *types, unsigned long type, const char *typename)
{
	rvmreg_t rs;
	rstring_t *s;

	s = r_string_create_from_ansistr(typename);
	r_gc_add(jse->cpu->gc, (robject_t*)s);
	rvm_reg_setstring(&rs, s);
	r_map_add_l(types, type, &rs);
}


static void rjs_engine_inittypes(rjs_engine_t *jse)
{
	rmap_t *gmap = (rmap_t *)RVM_CPUREG_GETP(jse->cpu, GP);
	rmap_t *types;
	rvmreg_t rt;

	types = r_map_create(sizeof(rvmreg_t), 3);
	r_gc_add(jse->cpu->gc, (robject_t*)types);
	rvm_reg_setjsobject(&rt, (robject_t *)types);
	r_map_add_l(gmap, RJS_GPKEY_TYPES, &rt);
	rjs_engine_addtypename(jse, types, RVM_DTYPE_UNDEF, "undefined");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_BOOLEAN, "boolean");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_DOUBLE, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_UNSIGNED, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_SIGNED, "number");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_STRING, "string");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_FUNCTION, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_NAN, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_MAP, "object");
	rjs_engine_addtypename(jse, types, RVM_DTYPE_POINTER, "object");
}


static void rjs_engine_initgp(rjs_engine_t *jse)
{
	rvmreg_t gp;
	rmap_t *gmap;

	rvm_reg_init(&gp);
	gmap = r_map_create(sizeof(rvmreg_t), 7);
	r_gc_add(jse->cpu->gc, (robject_t*)gmap);
	rvm_reg_setjsobject(RVM_CPUREG_PTR(jse->cpu, GP), (robject_t *)gmap);
	rjs_engine_inittypes(jse);
}


int rjs_engine_open(rjs_engine_t *jse)
{
	return 0;
}


int rjs_engine_addswitable(rjs_engine_t *jse, const char *tabname, rvm_switable_t *switalbe)
{
	return rvm_cpu_addswitable(jse->cpu, tabname, switalbe);
}


static int rjs_engine_parse(rjs_engine_t *jse, const char *script, unsigned long size, rarray_t *records, rjs_error_t *error)
{
	long res = 0;

	res = rjs_parser_exec(jse->pa, script, size, records, error);
	return res;
}


int rjs_engine_compile(rjs_engine_t *jse, const char *script, unsigned long size)
{
	rvm_codegen_t *topcg = NULL;
	rarray_t *records = rpa_records_create();
	rjs_error_t error;
	jse->co->debug = jse->debugcompile;

	r_memset(&error, 0, sizeof(error));
	if (rjs_engine_parse(jse, script, size, records, &error) < 0) {

		goto err;
	}

	topcg =  r_array_empty(jse->cgs) ? NULL : r_array_last(jse->cgs, rvm_codegen_t*);
	if (!topcg || (topcg->userdata & RJS_COMPILER_CODEGENKEEP) == 1) {
		topcg = rvm_codegen_create();
		r_array_add(jse->cgs, &topcg);
	} else {
		rvm_codegen_clear(topcg);
	}
	topcg->userdata = 0;
	if (rjs_compiler_compile(jse->co, script, size, records, topcg, &error) < 0) {
		topcg->userdata = 0;
		goto err;
	}

	rpa_records_destroy(records);
	return 0;

err:
	r_array_add(jse->errors, &error);
	rpa_records_destroy(records);
	return -1;
}


int rjs_engine_dumpast(rjs_engine_t *jse, const char *script, unsigned long size)
{
	rjs_error_t error;
	rarray_t *records = rpa_records_create();

	if (rjs_engine_parse(jse, script, size, records, &error) < 0) {


		return -1;
	}

	if (records) {
		long i;
		for (i = 0; i < rpa_records_length(records); i++)
			rpa_record_dump(records, i);
	}

	rpa_records_destroy(records);
	return 0;
}


int rjs_engine_compile_s(rjs_engine_t *jse, const char *script)
{
	return rjs_engine_compile(jse, script, r_strlen(script));
}


int rjs_engine_close(rjs_engine_t *jse)
{

	return 0;
}


int rjs_engine_run(rjs_engine_t *jse)
{
	int res = 0;
	rvm_codegen_t *cg = r_array_empty(jse->cgs) ? NULL : r_array_last(jse->cgs, rvm_codegen_t*);

	if (!cg) {

		return -1;
	}
	if (jse->debugexec) {
		res = rvm_cpu_exec_debug(jse->cpu, rvm_codegen_getcode(cg, 0), 0);
	} else {
		res = rvm_cpu_exec(jse->cpu, rvm_codegen_getcode(cg, 0), 0);
	}

	if (jse->cpu->error == RVM_E_USERABORT) {
		rword idx = RVM_CPUREG_GETIP(jse->cpu, PC) - rvm_codegen_getcode(cg, 0);
		if (idx >= 0) {
			r_printf("Aborted at source index: %ld\n", rvm_codegen_getsource(cg, (unsigned long)idx));
		}
	}
	return res;
}


rvmreg_t * rjs_engine_exec(rjs_engine_t *jse, const char *script, unsigned long size)
{
	if (rjs_engine_compile(jse, script, size) < 0)
		return NULL;
	RVM_CPUREG_SETU(jse->cpu, FP, 0);
	RVM_CPUREG_SETU(jse->cpu, SP, 0);
	if (rjs_engine_run(jse) < 0)
		return NULL;
	return RVM_CPUREG_PTR(jse->cpu, R0);
}


rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const char *script)
{
	return rjs_engine_exec(jse, script, r_strlen(script));
}


static int rjs_compiler_argarray_setup(rjs_compiler_t *co)
{
	rvm_varmap_t *v;
	rvmreg_t count = rvm_reg_create_signed(0);
	rmap_t *a;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		return -1;
	}
	a = r_map_create(sizeof(rvmreg_t), 7);
	r_gc_add(co->cpu->gc, (robject_t*)a);
	r_map_add_s(a, "count", &count);
	rvm_reg_setjsobject((rvmreg_t*)v->data.ptr, (robject_t*)a);
	return 0;
}


static int rjs_compiler_addarg(rjs_compiler_t *co, rvmreg_t *arg)
{
	rvm_varmap_t *v;
	rmap_t *a;
	rvmreg_t *count;
	long index;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		return -1;
	}
	a = (rmap_t*)RVM_REG_GETP((rvmreg_t*)v->data.ptr);
	index = r_map_lookup_s(a, -1, "count");
	R_ASSERT(index >= 0);
	count = (rvmreg_t *)r_map_value(a, index);
	r_map_add_l(a, (long)RVM_REG_GETL(count), arg);
	rvm_reg_setsigned(count, RVM_REG_GETL(count) + 1);

	return 0;
}


rvmreg_t *rjs_engine_vexec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, va_list args)
{
	rvmreg_t arg;
	unsigned long i = 0;

	if (rjs_engine_compile(jse, script, size) < 0)
		return NULL;
	if (nargs > 0) {
		rjs_compiler_argarray_setup(jse->co);
		for (i = 0; i < nargs; i++) {
			arg = va_arg(args, rvmreg_t);
			rjs_compiler_addarg(jse->co, &arg);
		}
	}
	RVM_CPUREG_SETU(jse->cpu, FP, 0);
	RVM_CPUREG_SETU(jse->cpu, SP, 0);
	if (rjs_engine_run(jse) < 0)
		return NULL;
	return RVM_CPUREG_PTR(jse->cpu, R0);
}


rvmreg_t *rjs_engine_args_exec(rjs_engine_t *jse, const char *script, unsigned long size, unsigned long nargs, ...)
{
	rvmreg_t *ret;
	va_list args;
	va_start(args, nargs);
	ret = rjs_engine_vexec(jse, script, size, nargs, args);
	va_end(args);
	return ret;
}


rvmreg_t *rjs_engine_args_exec_s(rjs_engine_t *jse, const char *script, unsigned long nargs, ...)
{
	rvmreg_t *ret;
	va_list args;
	va_start(args, nargs);
	ret = rjs_engine_vexec(jse, script, r_strlen(script), nargs, args);
	va_end(args);
	return ret;
}


static void rjs_engine_print(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		r_printf("%lu\n", RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_NAN)
		r_printf("NaN\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_UNDEF)
		r_printf("undefined\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_BOOLEAN)
		r_printf("%s\n", RVM_REG_GETU(r) ? "true" : "false");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_POINTER)
		r_printf("%p\n", RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SIGNED)
		r_printf("%ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("%f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("%s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_MAP)
		r_printf("(object) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_HARRAY)
		r_printf("(hashed array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}


static void rjs_engine_dbgprint(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		r_printf("(UNSIGNED) %lu\n", RVM_REG_GETU(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_NAN)
		r_printf("(NAN) NaN\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_UNDEF)
		r_printf("(UNDEF) undefined\n");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_BOOLEAN)
		r_printf("(BOOLEAN) %s\n", RVM_REG_GETU(r) ? "true" : "false");
	else if (rvm_reg_gettype(r) == RVM_DTYPE_POINTER)
		r_printf("(POINTER) %p\n", RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SIGNED)
		r_printf("(LONG) %ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("(DOUBLE) %f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("(STRING) %s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_MAP)
		r_printf("(object) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_HARRAY)
		r_printf("(hashed array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(swi function) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_FUNCTION)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}


static void rjs_engine_object(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}


void rjs_engine_abort(rjs_engine_t *jse, rjs_error_t *error)
{
	if (error) {
		r_array_add(jse->errors, error);
	}
	rvm_cpu_abort(jse->cpu);
}


rjs_engine_t *rjs_engine_get(rvmcpu_t *cpu)
{
	return (rjs_engine_t *)cpu->userdata1;
}
