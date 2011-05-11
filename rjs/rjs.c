#include "rmem.h"
#include "rjs.h"


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
	rjs_engine_t *jse = (rjs_engine_t *) r_zmalloc(sizeof(*jse));

	jse->pa = rjs_parser_create();
	jse->cpu = rvm_cpu_create_default();
	rvm_cpu_addswitable(jse->cpu, "rjsswitable", rjsswitable);
	return jse;
}


void rjs_engine_destroy(rjs_engine_t *jse)
{
	if (jse) {
		rjs_parser_destroy(jse->pa);
		rvm_cpu_destroy(jse->cpu);
		rjs_compiler_destroy(jse->co);
		r_free(jse);
	}
}


rint rjs_engine_open(rjs_engine_t *jse)
{
	return 0;
}


static rint rjs_engine_parse(rjs_engine_t *jse, const rchar *script, rsize_t size, rarray_t **records)
{
	rlong res = 0;
	res = rjs_parser_exec(jse->pa, script, size, records);
	return res;
}


rint rjs_engine_compile(rjs_engine_t *jse, const rchar *script, rsize_t size)
{
	rarray_t *records = NULL;
	if (jse->co)
		rjs_compiler_destroy(jse->co);
	jse->co = rjs_compiler_create(jse->cpu);
	jse->co->debug = jse->debugcompile;

	if (rjs_engine_parse(jse, script, size, &records) < 0) {

		goto err;
	}

	if (rjs_compiler_compile(jse->co, records) < 0) {

		goto err;
	}

	r_array_destroy(records);
	return 0;

err:
	r_array_destroy(records);
	return -1;
}


rint rjs_engine_dumpast(rjs_engine_t *jse, const rchar *script, rsize_t size)
{
	rarray_t *records = NULL;
	if (rjs_engine_parse(jse, script, size, &records) < 0) {


		return -1;
	}

	if (records) {
		rlong i;
		for (i = 0; i < r_array_length(records); i++)
			rpa_record_dump(records, i);
	}

	r_array_destroy(records);
	return 0;
}


rint rjs_engine_compile_s(rjs_engine_t *jse, const rchar *script)
{
	return rjs_engine_compile(jse, script, r_strlen(script));
}


rint rjs_engine_close(rjs_engine_t *jse)
{

	return 0;
}


rint rjs_engine_run(rjs_engine_t *jse)
{
	rint res = 0;

	if (!jse->co) {

		return -1;
	}
	if (jse->debugexec) {
		res = rvm_cpu_exec_debug(jse->cpu, rvm_codegen_getcode(jse->co->cg, 0), 0);
	} else {
		res = rvm_cpu_exec(jse->cpu, rvm_codegen_getcode(jse->co->cg, 0), 0);
	}
	return res;
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
	else if (rvm_reg_gettype(r) == RVM_DTYPE_JSOBJECT)
		r_printf("(object) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_HARRAY)
		r_printf("(hashed array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}


static void rjs_engine_object(rvmcpu_t *cpu, rvm_asmins_t *ins)
{

}
