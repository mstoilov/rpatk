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
	rpa_dbex_handle dbex;
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	ruint fpoff;
	ruint dectip;
} rvm_compiler_t;


rvm_compiler_t *rvm_compiler_create(rpa_dbex_handle dbex)
{
	rvm_compiler_t *co;

	co = r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->fpoff = 1;
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	co->dbex = dbex;
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
//#ifdef DEBUGPRINT
	fprintf(stdout, "%s: ", name);
	fwrite(input, sizeof(char), size, stdout);
	fprintf(stdout, "\n");
//#endif
	return size;
}


void codegen_dump_code(rvm_asmins_t *code, rulong size)
{
#ifdef DEBUGPRINT
	rvm_asm_dump(code, size);
#endif
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
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

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
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

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
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

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
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_pop_r0_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_integer_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, DA, XX, XX, r_strtol(input, NULL, 10)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_double_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmd(RVM_PUSH, DA, XX, XX, r_strtod(input, NULL)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_string_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, (void*)input));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R2, DA, XX, size));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCSTR, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));


	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}

int codegen_program_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rint i;
	ruint off;

	rvm_codegen_insertins(co->cg, 0, rvm_asm(RVM_ADD, SP, SP, DA, co->fpoff));
	rvm_codegen_insertins(co->cg, 0, rvm_asm(RVM_MOV, FP, SP, XX, 0));

	off = rvm_codegen_getcodesize(co->cg);
	for (i = 1; i < co->fpoff; i++) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CLS, R1, FP, DA, i));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R1, FP, DA, i));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R1, XX, XX, 0));
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opassign_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R0, FP, R1, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_var_push_val_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);

	if (!v) {
		fprintf(stdout, "ERROR: undefined variable: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, v->data.offset));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_var_push_fpoff_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);

	if (!v) {
		fprintf(stdout, "ERROR: undefined variable: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, DA, XX, XX, v->data.offset));
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varalloc_push_fpoff_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);
	if (v) {
		fprintf(stdout, "ERROR: variable already defined: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	rvm_scope_addoffset(co->scope, input, size, co->fpoff);
	co->dectip += 1;
	co->fpoff += 1;
	v = rvm_scope_tiplookup(co->scope, input, size);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, DA, XX, XX, v->data.offset));
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varalloc_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);
	if (v) {
		fprintf(stdout, "ERROR: variable already defined: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}


	rvm_scope_addoffset(co->scope, input, size, co->fpoff);
	co->dectip += 1;
	co->fpoff += 1;

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_compile_error_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	fprintf(stdout, "COMPILE ERROR pos: %ld\n", (long) (input - start));
	rpa_dbex_abort(co->dbex);
	return 0;
}


int main(int argc, char *argv[])
{
	int res;
	rvmcpu_t *cpu;
	ruint ntable;
	rpa_dbex_handle dbex = rpa_dbex_create();
	rvm_compiler_t *co = rvm_compiler_create(dbex);


	rpa_dbex_add_callback_exact(dbex, "mulop", RPA_REASON_MATCHED, codegen_mulop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "divop", RPA_REASON_MATCHED, codegen_divop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "addop", RPA_REASON_MATCHED, codegen_addop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "subop", RPA_REASON_MATCHED, codegen_subop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "integer", RPA_REASON_MATCHED, codegen_integer_callback, co);
	rpa_dbex_add_callback_exact(dbex, "double", RPA_REASON_MATCHED, codegen_double_callback, co);
	rpa_dbex_add_callback_exact(dbex, "program", RPA_REASON_MATCHED, codegen_program_callback, co);
	rpa_dbex_add_callback_exact(dbex, "varinit", RPA_REASON_MATCHED, codegen_opassign_callback, co);
	rpa_dbex_add_callback_exact(dbex, "varassign", RPA_REASON_MATCHED, codegen_opassign_callback, co);
	rpa_dbex_add_callback_exact(dbex, "var_push_val", RPA_REASON_MATCHED, codegen_var_push_val_callback, co);
	rpa_dbex_add_callback_exact(dbex, "var_push_fpoff", RPA_REASON_MATCHED, codegen_var_push_fpoff_callback, co);
	rpa_dbex_add_callback_exact(dbex, "varalloc_push_fpoff", RPA_REASON_MATCHED, codegen_varalloc_push_fpoff_callback, co);
	rpa_dbex_add_callback_exact(dbex, "pop_r0", RPA_REASON_MATCHED, codegen_pop_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "varalloc", RPA_REASON_MATCHED, codegen_varalloc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "compile_error", RPA_REASON_MATCHED, codegen_compile_error_callback, co);


	rpa_dbex_open(dbex);
	rpa_dbex_load_string(dbex, "S					::= ( [#x20] | [#x9] | [#xD] | [#xA] )*");
	rpa_dbex_load_string(dbex, "C					::= <S>? ',' <S>?");
	rpa_dbex_load_string(dbex, "SC					::= <S>? ';' <S>?");
	rpa_dbex_load_string(dbex, "EQ					::= <S>? '=' <S>?");
	rpa_dbex_load_string(dbex, "PLUS				::= <S>? '+' <S>?");
	rpa_dbex_load_string(dbex, "MIN					::= <S>? '-' <S>?");
	rpa_dbex_load_string(dbex, "AST					::= <S>? '*' <S>?");
	rpa_dbex_load_string(dbex, "SLASH				::= <S>? '/' <S>?");
	rpa_dbex_load_string(dbex, "digit				::= [0-9]");
	rpa_dbex_load_string(dbex, "eoe					::= (';' | [#xD] | [#xA])+");
	rpa_dbex_load_string(dbex, "varname				::= [a-zA-z][a-zA-z0-9]*");
	rpa_dbex_load_string(dbex, "var_push_val		::= <:varname:>");
	rpa_dbex_load_string(dbex, "var_push_fpoff		::= <:varname:>");
	rpa_dbex_load_string(dbex, "varalloc_push_fpoff	::= <:varname:>");
	rpa_dbex_load_string(dbex, "varalloc			::= <:varname:>");
	rpa_dbex_load_string(dbex, "varspec				::= <:varinit:> | <:varalloc:>");
	rpa_dbex_load_string(dbex, "varinit				::= <:varalloc_push_fpoff:> <:EQ:> (<:varassign:> | <:pop_r0:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "varassign			::= <:var_push_fpoff:> <:EQ:> (<:varassign:> | <:pop_r0:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "declaration			::= 'var' (<:S:> <:varspec:> (<:C:> <:varspec:>)* <:SC:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "assignment			::= <:varassign:> <:SC:>");
//	rpa_dbex_load_string(dbex, "integer				::= <:digit:>+");
	rpa_dbex_load_string(dbex, "integer				::= <loopyinteger>");
	rpa_dbex_load_string(dbex, "loopyinteger		::= <loopyinteger><digit>|<digit>");
	rpa_dbex_load_string(dbex, "double				::= <integer> '.' <integer> | '.' <integer>");
	rpa_dbex_load_string(dbex, "term				::= '('<:S:>? <:expr:> <:S:>? ')' | <:var_push_val:> | <:double:> | <:integer:>");
	rpa_dbex_load_string(dbex, "mulop				::= <:mulex:> <:AST:> (<:term:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "divop				::= <:mulex:> <:SLASH:> (<:term:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "mulex				::= <:mulop:> | <:divop:> | <:term:>");
	rpa_dbex_load_string(dbex, "addop				::= <:expr:> <:PLUS:> <:mulex:> ");
	rpa_dbex_load_string(dbex, "subop				::= <:expr:> <:MIN:> (<:mulex:> | <;compile_error;>)");
	rpa_dbex_load_string(dbex, "addop1    			::= <:addop:>");
	rpa_dbex_load_string(dbex, "addop2    			::= <:addop1:>");
	rpa_dbex_load_string(dbex, "addop3    			::= <:addop2:>");
	rpa_dbex_load_string(dbex, "addop4    			::= <:addop3:>");
	rpa_dbex_load_string(dbex, "addop5    			::= <:addop4:>");
	rpa_dbex_load_string(dbex, "addop6    			::= <:addop5:>");
	rpa_dbex_load_string(dbex, "addop7    			::= <:addop6:>");
	rpa_dbex_load_string(dbex, "addop8    			::= <:addop7:>");
	rpa_dbex_load_string(dbex, "addop9    			::= <:addop8:>");

	rpa_dbex_load_string(dbex, "expr    			::= <:addop9:> | <:subop:> | <:mulex:>");
	rpa_dbex_load_string(dbex, "pop_r0				::= <:expr:>");
	rpa_dbex_load_string(dbex, "compile_error		::= .");
	rpa_dbex_load_string(dbex, "program				::= ((<:declaration:>|<:assignment:>)* <:S:>? (<:pop_r0:><:SC:>)*) | <;compile_error;>");
	rpa_dbex_load_string(dbex, "exec				::= <:program:>");

	rpa_dbex_close(dbex);
	if (argc > 1) {
		res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), argv[1], argv[1], argv[1] + r_strlen(argv[1]));
		if (res <= 0)
			rvm_codegen_clear(co->cg);
	}
	rpa_dbex_destroy(dbex);
	rvm_relocate(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
//	codegen_dump_code(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
	cpu = rvm_cpu_create();
	ntable = rvmcpu_switable_add(cpu, switable);
	if (rvm_codegen_getcodesize(co->cg)) {
		fprintf(stdout, "\nExecution:\n");
#ifdef DEBUGPRINT
		rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(co->cg, 0), 0);
#else
		rvm_cpu_exec(cpu, rvm_codegen_getcode(co->cg, 0), 0);
#endif
	}

	rvm_cpu_destroy(cpu);
	rvm_compiler_destroy(co);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());

	return 0;
}
