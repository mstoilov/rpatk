#include <stdarg.h>
#include "rlib/rmem.h"
#include "rlib/rmap.h"
#include "rjs/rjs.h"
#include "rvm/rvmcodegen.h"


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


const rchar *rjs_version()
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
	rlong i;
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


rinteger rjs_engine_open(rjs_engine_t *jse)
{
	return 0;
}


rinteger rjs_engine_addswitable(rjs_engine_t *jse, const rchar *tabname, rvm_switable_t *switalbe)
{
	return rvm_cpu_addswitable(jse->cpu, tabname, switalbe);
}


static rinteger rjs_engine_parse(rjs_engine_t *jse, const rchar *script, rsize_t size, rarray_t *records, rjs_error_t *error)
{
	rlong res = 0;

	res = rjs_parser_exec(jse->pa, script, size, records, error);
	return res;
}


rinteger rjs_engine_compile(rjs_engine_t *jse, const rchar *script, rsize_t size)
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


rinteger rjs_engine_dumpast(rjs_engine_t *jse, const rchar *script, rsize_t size)
{
	rjs_error_t error;
	rarray_t *records = rpa_records_create();

	if (rjs_engine_parse(jse, script, size, records, &error) < 0) {


		return -1;
	}

	if (records) {
		rlong i;
		for (i = 0; i < rpa_records_length(records); i++)
			rpa_record_dump(records, i);
	}

	rpa_records_destroy(records);
	return 0;
}


rinteger rjs_engine_compile_s(rjs_engine_t *jse, const rchar *script)
{
	return rjs_engine_compile(jse, script, r_strlen(script));
}


rinteger rjs_engine_close(rjs_engine_t *jse)
{

	return 0;
}


rinteger rjs_engine_run(rjs_engine_t *jse)
{
	rinteger res = 0;
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
		rlong idx = RVM_CPUREG_GETIP(jse->cpu, PC) - rvm_codegen_getcode(cg, 0);
		if (idx >= 0) {
			r_printf("Aborted at source index: %ld\n", rvm_codegen_getsource(cg, idx));
		}
	}

	return res;
}


rvmreg_t * rjs_engine_exec(rjs_engine_t *jse, const rchar *script, rsize_t size)
{
	if (rjs_engine_compile(jse, script, size) < 0)
		return NULL;
	RVM_CPUREG_SETU(jse->cpu, FP, 0);
	RVM_CPUREG_SETU(jse->cpu, SP, 0);
	if (rjs_engine_run(jse) < 0)
		return NULL;
	return RVM_CPUREG_PTR(jse->cpu, R0);
}


rvmreg_t *rjs_engine_exec_s(rjs_engine_t *jse, const rchar *script)
{
	return rjs_engine_exec(jse, script, r_strlen(script));
}


static rinteger rjs_compiler_argarray_setup(rjs_compiler_t *co)
{
	rvm_varmap_t *v;
	rvmreg_t count = rvm_reg_create_long(0);
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


static rinteger rjs_compiler_addarg(rjs_compiler_t *co, rvmreg_t *arg)
{
	rvm_varmap_t *v;
	rmap_t *a;
	rvmreg_t *count;
	rlong index;

	v = rvm_scope_tiplookup_s(co->scope, "ARGS");
	if (!v) {
		return -1;
	}
	a = (rmap_t*)RVM_REG_GETP((rvmreg_t*)v->data.ptr);
	index = r_map_lookup_s(a, -1, "count");
	R_ASSERT(index >= 0);
	count = (rvmreg_t *)r_map_value(a, index);
	r_map_add_l(a, RVM_REG_GETL(count), arg);
	rvm_reg_setlong(count, RVM_REG_GETL(count) + 1);

	return 0;
}


rvmreg_t *rjs_engine_vexec(rjs_engine_t *jse, const rchar *script, rsize_t size, rsize_t nargs, va_list args)
{
	rvmreg_t arg;
	rsize_t i = 0;

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


rvmreg_t *rjs_engine_args_exec(rjs_engine_t *jse, const rchar *script, rsize_t size, rsize_t nargs, ...)
{
	rvmreg_t *ret;
	va_list args;
	va_start(args, nargs);
	ret = rjs_engine_vexec(jse, script, size, nargs, args);
	va_end(args);
	return ret;
}


rvmreg_t *rjs_engine_args_exec_s(rjs_engine_t *jse, const rchar *script, rsize_t nargs, ...)
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
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		r_printf("%ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("%f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("%s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_JSOBJECT)
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
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		r_printf("(LONG) %ld\n", RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		r_printf("(DOUBLE) %f\n", RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		r_printf("(STRING) %s\n", ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRPTR)
		r_printf("(STRPTR) %s\n", (RVM_REG_GETSTR(r)));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_JSOBJECT)
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
