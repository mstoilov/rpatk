#include "rmem.h"
#include "rjs.h"


const rchar *rjs_version()
{
	return RJS_VERSION_STRING;
}


rjs_engine_t *rjs_engine_create()
{
	rjs_engine_t *jse = (rjs_engine_t *) r_zmalloc(sizeof(*jse));

	jse->pa = rjs_parser_create();
	jse->cpu = rvm_cpu_create_default();
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
