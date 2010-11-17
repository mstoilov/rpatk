#include <stdio.h>
#include <stdlib.h>
#include "rmem.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"
#include "rvmscope.h"


typedef struct rvm_vardeclaration_s {
	const rchar *varname;
	ruint varnamesiz;
	const rchar *val;
	ruint valsiz;
} rvm_vardeclaration_t;


typedef struct rvm_compiler_s {
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	ruint fpoff;
	ruint dectip;
} rvm_compiler_t;


void rvm_compiler_push_r(rvm_compiler_t *co, ruint r)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, r, XX, XX, 0));
}


void rvm_compiler_pop_r(rvm_compiler_t *co, ruint r)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, r, XX, XX, 0));
}


void rvm_compiler_load_fpoff(rvm_compiler_t *co, ruint r, ruint off)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, r, FP, off, 0));
}


void rvm_compiler_store_fpoff(rvm_compiler_t *co, ruint r, ruint off)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, r, FP, off, 0));
}


int rvm_compiler_name_to_fpoff(rvm_compiler_t *co, ruint r, const rchar *name, ruint size)
{
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope,name, size);
	if (!v) {
		return -1;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, r, DA, XX, v->data.offset));
	return 0;
}


void rvm_compiler_varcreate(rvm_compiler_t *co, const rchar *name, ruint size)
{
	rvm_scope_addoffset(co->scope, name, size, co->fpoff);
	co->dectip += 1;
	co->fpoff += 1;
}


int rvm_compiler_pushvar(rvm_compiler_t *co, const rchar *name, ruint size)
{
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope,name, size);
	if (!v) {
		fprintf(stdout, "undefined variable: ");
		fwrite(name, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		return -1;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, DA, v->data.offset));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	return 0;
}


int rvm_compiler_varoffset_r4(rvm_compiler_t *co, const rchar *name, ruint size)
{
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope,name, size);
	if (!v) {
		fprintf(stdout, "undefined variable: ");
		fwrite(name, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		return -1;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R4, DA, XX, v->data.offset));
	return 0;
}


int rvm_compiler_varstore_r0(rvm_compiler_t *co, const rchar *name, ruint size)
{

	rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R0, FP, R4, 0));
	return 0;
}


int rvm_compiler_varload_r0(rvm_compiler_t *co, const rchar *name, ruint size)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, R4, 0));
	return 0;
}


rvm_compiler_t *rvm_compiler_create()
{
	rvm_compiler_t *co;

	co = r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->fpoff = 1;
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	return co;
}


void rvm_compiler_destroy(rvm_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
		r_free(co);
	}
}


static void test_swi_none(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
}

static rvm_switable_t switable[] = {
		{"none", test_swi_none},
		{NULL, NULL},
};

#define DEBUGPRINT 1
int codegen_print_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
#ifdef DEBUGPRINT
	fprintf(stdout, "%s: ", name);
	fwrite(input, sizeof(char), size, stdout);
	fprintf(stdout, "\n");
#endif
	return size;
}


int codegen_mulop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EMUL, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_divop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EDIV, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;

}


int codegen_subop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ESUB, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_addop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EADD, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_resexpr_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_integer_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, DA, XX, XX, r_strtol(input, NULL, 10)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_double_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmd(RVM_PUSH, DA, XX, XX, r_strtod(input, NULL)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_program_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_insertins(co->cg, 0, rvm_asm(RVM_ADD, SP, SP, DA, co->fpoff));
	rvm_codegen_insertins(co->cg, 0, rvm_asm(RVM_MOV, FP, SP, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varname_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	if (!rvm_scope_lookup(co->scope, input, size))
		rvm_compiler_varcreate(co, input, size);
	rvm_compiler_name_to_fpoff(co, R0, input, size);
	rvm_compiler_push_r(co, R0);
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}



int codegen_opassign_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R2, FP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, R2, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varvalue_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	rvm_asm_dump(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int main(int argc, char *argv[])
{
	int res;
	rvmcpu_t *cpu;
	ruint ntable;
	rvm_compiler_t *co = rvm_compiler_create();
	rpa_dbex_handle dbex;

	dbex = rpa_dbex_create();
	rpa_dbex_add_callback(dbex, "mulop", RPA_REASON_MATCHED, codegen_mulop_callback, co);
	rpa_dbex_add_callback(dbex, "divop", RPA_REASON_MATCHED, codegen_divop_callback, co);
	rpa_dbex_add_callback(dbex, "addop", RPA_REASON_MATCHED, codegen_addop_callback, co);
	rpa_dbex_add_callback(dbex, "subop", RPA_REASON_MATCHED, codegen_subop_callback, co);
	rpa_dbex_add_callback(dbex, "integer", RPA_REASON_MATCHED, codegen_integer_callback, co);
	rpa_dbex_add_callback(dbex, "double", RPA_REASON_MATCHED, codegen_double_callback, co);
	rpa_dbex_add_callback(dbex, "program", RPA_REASON_MATCHED, codegen_program_callback, co);
	rpa_dbex_add_callback(dbex, "assignment", RPA_REASON_MATCHED, codegen_opassign_callback, co);
	rpa_dbex_add_callback(dbex, "varname", RPA_REASON_MATCHED, codegen_varname_callback, co);
	rpa_dbex_add_callback(dbex, "varname", RPA_REASON_MATCHED, codegen_varname_callback, co);
	rpa_dbex_add_callback(dbex, "varvalue", RPA_REASON_MATCHED, codegen_varvalue_callback, co);
	rpa_dbex_add_callback(dbex, "resexpr", RPA_REASON_MATCHED, codegen_resexpr_callback, co);


	rpa_dbex_open(dbex);
	rpa_dbex_load_string(dbex, "S			::= ( [#x20] | [#x9] | [#xD] | [#xA] )*");
	rpa_dbex_load_string(dbex, "C			::= <S>? ',' <S>?");
	rpa_dbex_load_string(dbex, "SC			::= <S>? ';' <S>?");
	rpa_dbex_load_string(dbex, "digit		::= [0-9]");
	rpa_dbex_load_string(dbex, "eoe			::= (';' | [#xD] | [#xA])+");
	rpa_dbex_load_string(dbex, "varname		::= [a-zA-z][a-zA-z0-9]*");
	rpa_dbex_load_string(dbex, "varspec		::= <:varassign:> | <:varname:>");
	rpa_dbex_load_string(dbex, "varassign	::= <:varname:> <:S:>? '=' <:S:>? <:expr:>");
	rpa_dbex_load_string(dbex, "varvalue	::= <:varname:>");

	rpa_dbex_load_string(dbex, "assignment	::= <:S:>? <:varname:> <:S:>? '=' <:S:>? (<double> | <integer>) <:S:>? <:eoe:>");
	rpa_dbex_load_string(dbex, "declaration	::= 'var' <:S:> <:varspec:> (<:C:> <:varspec:>)* <:SC:>");
	rpa_dbex_load_string(dbex, "integer		::= <:digit:>+");
	rpa_dbex_load_string(dbex, "double		::= <integer> '.' <integer> | '.' <integer>");

	rpa_dbex_load_string(dbex, "term		::= <:varvalue:> | <:double:> | <:integer:> | '('<:S:>? <:expr:> <:S:>? ')'");
	rpa_dbex_load_string(dbex, "mulop		::= <:mulex:> <:S:>? '*' <:S:>? <:term:>");
	rpa_dbex_load_string(dbex, "divop		::= <:mulex:> <:S:>? '/' <:S:>? <:term:>");
	rpa_dbex_load_string(dbex, "mulex		::= <:mulop:> | <:divop:> | <:term:>");
	rpa_dbex_load_string(dbex, "addop		::= <:expr:> <:S:>? '+' <:S:>? <:mulex:>");
	rpa_dbex_load_string(dbex, "subop		::= <:expr:> <:S:>? '-' <:S:>? <:mulex:>");
	rpa_dbex_load_string(dbex, "expr    	::= <:addop:> | <:subop:> | <:mulex:>");
	rpa_dbex_load_string(dbex, "resexpr		::= <:expr:>");
	rpa_dbex_load_string(dbex, "program		::= (<:declaration:>|<:assignment:>)* <:S:>? <:resexpr:>");
	rpa_dbex_load_string(dbex, "exec		::= <:program:>");

	rpa_dbex_close(dbex);
	if (argc > 1) {
		res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), argv[1], argv[1], argv[1] + r_strlen(argv[1]));
	}
	/* Temporary */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	rpa_dbex_destroy(dbex);
	rvm_relocate(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
	rvm_asm_dump(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));


	cpu = rvm_cpu_create();
	ntable = rvmcpu_switable_add(cpu, switable);
	if (rvm_codegen_getcodesize(co->cg))
		rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(co->cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_compiler_destroy(co);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());

	return 0;
}
