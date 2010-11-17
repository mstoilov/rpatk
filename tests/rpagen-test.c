#include <stdio.h>
#include <stdlib.h>
#include "rmem.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"


static void test_swi_none(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
}

static rvm_switable_t switable[] = {
		{"none", test_swi_none},
		{NULL, NULL},
};


int codegen_mulop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_EMUL, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	return size;
}


int codegen_divop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_EDIV, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	return size;

}


int codegen_subop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_ESUB, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	return size;
}


int codegen_addop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_EADD, R0, R1, R2, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	return size;
}


int codegen_integer_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_PUSH, DA, XX, XX, r_strtol(input, NULL, 10)));
	return size;
}


int codegen_double_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asmd(RVM_PUSH, DA, XX, XX, r_strtod(input, NULL)));
	return size;
}


int codegen_allexpr_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_codegen_t *cg = (rvm_codegen_t *)userdata;
	rvm_codegen_addins(cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	return size;
}


int main(int argc, char *argv[])
{
	int res;
	rvmcpu_t *cpu;
	rvm_codegen_t *cg;
	ruint ntable;

	rpa_dbex_handle dbex;

	cg = rvm_codegen_create();
	dbex = rpa_dbex_create();
	rpa_dbex_add_callback(dbex, "mulop", RPA_REASON_MATCHED, codegen_mulop_callback, cg);
	rpa_dbex_add_callback(dbex, "divop", RPA_REASON_MATCHED, codegen_divop_callback, cg);
	rpa_dbex_add_callback(dbex, "addop", RPA_REASON_MATCHED, codegen_addop_callback, cg);
	rpa_dbex_add_callback(dbex, "subop", RPA_REASON_MATCHED, codegen_subop_callback, cg);
	rpa_dbex_add_callback(dbex, "integer", RPA_REASON_MATCHED, codegen_integer_callback, cg);
	rpa_dbex_add_callback(dbex, "double", RPA_REASON_MATCHED, codegen_double_callback, cg);
	rpa_dbex_add_callback(dbex, "allexpr", RPA_REASON_MATCHED, codegen_allexpr_callback, cg);

	rpa_dbex_open(dbex);
	rpa_dbex_load_string(dbex, "S		::= [#x20]+");
	rpa_dbex_load_string(dbex, "digit	::= [0-9]");
	rpa_dbex_load_string(dbex, "integer	::= <:digit:>+");
	rpa_dbex_load_string(dbex, "double	::= <:digit:>+ '.' <:digit:>+ | '.' <:digit:>+");
	rpa_dbex_load_string(dbex, "term	::= <:double:> | <:integer:> | '('<:S:>? <:expr:> <:S:>? ')'");
	rpa_dbex_load_string(dbex, "mulop	::= <:mulex:> <:S:>? '*' <:S:>? <:term:>");
	rpa_dbex_load_string(dbex, "divop	::= <:mulex:> <:S:>? '/' <:S:>? <:term:>");
	rpa_dbex_load_string(dbex, "mulex	::= <:mulop:> | <:divop:> | <:term:>");
	rpa_dbex_load_string(dbex, "addop	::= <:expr:> <:S:>? '+' <:S:>? <:mulex:>");
	rpa_dbex_load_string(dbex, "subop	::= <:expr:> <:S:>? '-' <:S:>? <:mulex:>");
	rpa_dbex_load_string(dbex, "expr    ::= <:addop:> | <:subop:> | <:mulex:>");
	rpa_dbex_load_string(dbex, "allexpr	::= <:expr:>");
	rpa_dbex_load_string(dbex, "exec	::= <:allexpr:>");
	rpa_dbex_close(dbex);
	if (argc > 1) {
		res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), argv[1], argv[1], argv[1] + r_strlen(argv[1]));
	}
	rpa_dbex_destroy(dbex);
	rvm_relocate(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));
	rvm_asm_dump(rvm_codegen_getcode(cg, 0), rvm_codegen_getcodesize(cg));


	cpu = rvm_cpu_create();
	ntable = rvmcpu_switable_add(cpu, switable);
	rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(cg, 0), 0);
	rvm_cpu_destroy(cpu);
	rvm_codegen_destroy(cg);
	fprintf(stdout, "Max alloc mem: %ld\n", r_debug_get_maxmem());
	fprintf(stdout, "Leaked mem: %ld\n", r_debug_get_allocmem());

	return 0;
}
