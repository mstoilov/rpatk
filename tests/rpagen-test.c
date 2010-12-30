#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rpaerror.h"
#include "rvmcodegen.h"
#include "rvmcodemap.h"
#include "rvmscope.h"
#include "rvmoperator.h"

#define DEBUGPRINT 1
static int debuginfo = 0;
static int parseinfo = 0;
static int verboseinfo = 0;

typedef struct rvm_vardeclaration_s {
	const rchar *varname;
	ruint varnamesiz;
	const rchar *val;
	ruint valsiz;
} rvm_vardeclaration_t;


typedef struct rvm_fundecl_s {
	const rchar *funname;
	rword funnamesiz;
	rword params;
	rword codestart;
	rword codesize;
} rvm_fundecl_t;


typedef struct rvm_funcall_s {
	const rchar *funname;
	rword funnamesiz;
	rword params;
} rvm_funcall_t;


typedef struct rvm_codespan_s {
	rword codestart;
	rword codesize;
} rvm_codespan_t;


typedef struct rvm_compiler_s {
	rpa_dbex_handle dbex;
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	rarray_t *fp;
	rarray_t *funcall;
	rarray_t *fundecl;
	rarray_t *opcodes;
	rarray_t *codespan;
	rvmcpu_t *cpu;
} rvm_compiler_t;

void rpagen_load_rules(rpa_dbex_handle dbex, rvm_compiler_t *co);


rvm_compiler_t *rvm_compiler_create(rpa_dbex_handle dbex)
{
	rvm_compiler_t *co;

	co = r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	co->opcodes = r_array_create(sizeof(ruint));
	co->fp = r_array_create(sizeof(rword));
	co->funcall = r_array_create(sizeof(rvm_funcall_t));
	co->fundecl = r_array_create(sizeof(rvm_fundecl_t));
	co->codespan = r_array_create(sizeof(rvm_codespan_t));
	r_array_push(co->fp, 0, rword);
	co->dbex = dbex;
	return co;
}


void rvm_compiler_destroy(rvm_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
		r_object_destroy((robject_t*) co->opcodes);
		r_object_destroy((robject_t*) co->fp);
		r_object_destroy((robject_t*) co->funcall);
		r_object_destroy((robject_t*) co->fundecl);
		r_object_destroy((robject_t*) co->codespan);
		r_free(co);
	}
}


static void test_swi_none(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
}


static void rvm_swi_negative(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *res = RVM_CPUREG_PTR(cpu, ins->op1);
	rvmreg_t *arg = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t zero;

	RVM_REG_CLEAR(&zero);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_SUB, cpu, res, &zero, arg);
}


static void rvm_swi_eadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *res = RVM_CPUREG_PTR(cpu, R0);
	rvmreg_t *arg1 = (rvmreg_t *)r_carray_slot(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);
	rvmreg_t *arg2 = (rvmreg_t *)r_carray_slot(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 2);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_ADD, cpu, res, arg1, arg2);
}

static void rvm_swi_eless(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *res = RVM_SPOFF_ADDR(cpu, 1);
	rvmreg_t *arg2 = RVM_SPOFF_ADDR(cpu, 0);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_LESS, cpu, res, res, arg2);
	RVM_CPUREG_SETU(cpu, SP, RVM_CPUREG_GETU(cpu, SP) - 1);
}


static void rvm_swi_bnonzero(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg = RVM_SPOFF_ADDR(cpu, 0);

	if (RVM_REG_GETU(arg))
		RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, PC) + RVM_CPUREG_GETU(cpu, ins->op1) - 1);
	RVM_CPUREG_SETU(cpu, SP, RVM_CPUREG_GETU(cpu, SP) - 1);
}


static void rvm_swi_identifiervalue(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	RVM_CPUREG_SET(cpu, ins->op1, *r_carray_rvmregslot(cpu->stack, RVM_CPUREG_GETU(cpu, ins->op2) + RVM_CPUREG_GETU(cpu, ins->op3)));
}


static rvm_switable_t switable[] = {
		{"none", test_swi_none},
		{"RVM_SWI_NEG", rvm_swi_negative},
		{"rvm_swi_eadd", rvm_swi_eadd},
		{"rvm_swi_eless", rvm_swi_eless},
		{"rvm_swi_bnonzero", rvm_swi_bnonzero},
		{"rvm_swi_identifiervalue", rvm_swi_identifiervalue},
		{NULL, NULL},
};


static void rvm_js_print(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = (rvmreg_t *)r_carray_slot(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);

	if (rvm_reg_gettype(r) == RVM_DTYPE_UNSIGNED)
		r_printf("%lu\n", RVM_REG_GETU(r));
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
	else if (rvm_reg_gettype(r) == RVM_DTYPE_ARRAY)
		r_printf("(array) %p\n",RVM_REG_GETP(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_SWIID)
		r_printf("(function) %p\n",RVM_REG_GETP(r));
	else
		r_printf("%p\n", RVM_REG_GETP(r));
}

static rvm_switable_t switable_js[] = {
		{"print", rvm_js_print},
		{NULL, NULL},
};

inline int codegen_print_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	if (parseinfo) {
		fprintf(stdout, "%s: ", name);
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		fflush(stdout);
	}

	return size;
}


void codegen_dump_code(rvm_asmins_t *code, rulong size)
{
	if (parseinfo)
		rvm_asm_dump(code, size);
}


int codegen_opcode_unary_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;

	if (r_strncmp(input, "-", size) == 0)
		r_array_push(co->opcodes, RVM_OPSWI(rvm_cpu_getswi_s(co->cpu, "RVM_SWI_NEG")), ruint);
	else
		r_array_push(co->opcodes, RVM_ABORT, ruint);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	return size;
}


int codegen_opcode_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;

	if (r_strncmp(input, "+", size) == 0)
		r_array_push(co->opcodes, RVM_EADD, ruint);
	else if (r_strncmp(input, "-", size) == 0)
		r_array_push(co->opcodes, RVM_ESUB, ruint);
	else if (r_strncmp(input, "*", size) == 0)
		r_array_push(co->opcodes, RVM_EMUL, ruint);
	else if (r_strncmp(input, "/", size) == 0)
		r_array_push(co->opcodes, RVM_EDIV, ruint);
	else if (r_strncmp(input, "%", size) == 0)
		r_array_push(co->opcodes, RVM_EMOD, ruint);
	else if (r_strncmp(input, "&&", size) == 0)
		r_array_push(co->opcodes, RVM_ELAND, ruint);
	else if (r_strncmp(input, "||", size) == 0)
		r_array_push(co->opcodes, RVM_ELOR, ruint);
	else if (r_strncmp(input, "&", size) == 0)
		r_array_push(co->opcodes, RVM_EAND, ruint);
	else if (r_strncmp(input, "|", size) == 0)
		r_array_push(co->opcodes, RVM_EORR, ruint);
	else if (r_strncmp(input, "^", size) == 0)
		r_array_push(co->opcodes, RVM_EXOR, ruint);
	else if (r_strncmp(input, ">>", size) == 0)
		r_array_push(co->opcodes, RVM_ELSR, ruint);
	else if (r_strncmp(input, "<<", size) == 0)
		r_array_push(co->opcodes, RVM_ELSL, ruint);
	else if (r_strncmp(input, ">>>", size) == 0)
		r_array_push(co->opcodes, RVM_ELSRU, ruint);
	else if (r_strncmp(input, "<=", size) == 0)
		r_array_push(co->opcodes, RVM_ELESSEQ, ruint);
	else if (r_strncmp(input, ">=", size) == 0)
		r_array_push(co->opcodes, RVM_EGREATEQ, ruint);
	else if (r_strncmp(input, "<", size) == 0)
		r_array_push(co->opcodes, RVM_ELESS, ruint);
	else if (r_strncmp(input, ">", size) == 0)
		r_array_push(co->opcodes, RVM_EGREAT, ruint);
	else if (r_strncmp(input, "===", size) == 0)
		r_array_push(co->opcodes, RVM_EEQ, ruint);
	else if (r_strncmp(input, "==", size) == 0)
		r_array_push(co->opcodes, RVM_EEQ, ruint);
	else if (r_strncmp(input, "!==", size) == 0)
		r_array_push(co->opcodes, RVM_ENOTEQ, ruint);
	else if (r_strncmp(input, "!=", size) == 0)
		r_array_push(co->opcodes, RVM_ENOTEQ, ruint);
	else if (r_strncmp(input, "!", size) == 0)
		r_array_push(co->opcodes, RVM_ELNOT, ruint);
	else if (r_strncmp(input, "~", size) == 0)
		r_array_push(co->opcodes, RVM_ENOT, ruint);
	else
		r_array_push(co->opcodes, RVM_ABORT, ruint);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	return size;
}


int codegen_binary_asmop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint opcode = r_array_pop(co->opcodes, ruint);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R1, R2, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_unary_asmop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint opcode = r_array_pop(co->opcodes, ruint);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_postfixexpression_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_TYPE, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, RVM_DTYPE_POINTER));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_push_r0_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_pop_r0_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_integer_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

//	rvm_codegen_addins(co->cg, rvm_asml(RVM_PUSH, DA, XX, XX, r_strtol(input, NULL, 10)));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strtol(input, NULL, 10)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_double_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

//	rvm_codegen_addins(co->cg, rvm_asmd(RVM_PUSH, DA, XX, XX, r_strtod(input, NULL)));
	rvm_codegen_addins(co->cg, rvm_asmd(RVM_MOV, R0, DA, XX, r_strtod(input, NULL)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_string_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, (void*)input));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R2, DA, XX, size));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCSTR, R0, R1, R2, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_programinit_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off;

	off = rvm_codegen_getcodesize(co->cg);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_program_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off;

	rvm_codegen_replaceins(co->cg, 0, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_replaceins(co->cg, 1, rvm_asm(RVM_ADD, SP, SP, DA, r_array_pop(co->fp, rword)));

	off = rvm_codegen_getcodesize(co->cg);
	if (debuginfo)
		rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opassign_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_postfixinc_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EADD, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_postfixdec_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ESUB, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_prefixinc_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EADD, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_prefixdec_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ESUB, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_dokeyword_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codemap_pushloopblock(co->cg->codemap, off, 0);
//	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_iterationdo_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_loopblock_t *loop = rvm_codemap_currentloopblock(co->cg->codemap);


	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 1));
	loop->size = rvm_codegen_getcodesize(co->cg) - loop->begin;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 0 - loop->size));
	rvm_codemap_poploopblock(co->cg->codemap);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_ptr_deref_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Address
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));	// Load the value from offset
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));	// Push it on the stack

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_arrayelementvalue_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Array
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ELDA, R0, R1, R0, 0));	// Load the value from array offset

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_memberexpressionbase_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_TYPE, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, RVM_DTYPE_POINTER));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R0, XX, 0)); 	// Array Address
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0)); 	// Array -> On Stack

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_arrayelementaddress_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly Array Address
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRA, R0, R1, R0, 0));	// Get the address of the element at offset R0

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}



int codegen_offset_to_ptr_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0)); 	// Offset
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CAST, R2, R2, DA, RVM_DTYPE_UNSIGNED)); 	// cast to unsigned
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Array
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRA, R0, R1, R2, 0));	// Pointer to element at offset
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));	// Push it on the stack

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_identifiervalue_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_lookup(co->scope, input, size);

	if (!v) {
		fprintf(stdout, "ERROR: undefined variable: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	if (v->datatype == VARMAP_DATATYPE_FPOFFSET) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ELDS, R0, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ELDS, R0, DA, XX, v->data.offset));
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));	// Push it on the stack

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_swiidexist_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rint swi = rvm_cpu_getswi(co->cpu, input, size);

	if (swi < 0)
		return 0;


	return size;
}


int codegen_swiid_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rint swi = rvm_cpu_getswi(co->cpu, input, size);

	if (swi < 0)
		return 0;

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, swi));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_SWIID));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opidentifier_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_lookup(co->scope, input, size);

	if (v) {
		if (v->datatype == VARMAP_DATATYPE_FPOFFSET) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, FP, DA, v->data.offset));
		} else {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, DA, XX, v->data.offset));
		}
		codegen_print_callback(name, userdata, input, size, reason, start, end);
		codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

		return size;
	}

	fprintf(stdout, "ERROR: undefined variable: ");
	fwrite(input, sizeof(char), size, stdout);
	fprintf(stdout, "\n");
	rpa_dbex_abort(co->dbex);
	return 0;

}


int codegen_varalloc_to_ptr_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);
	if (v) {
		fprintf(stdout, "ERROR: variable already defined: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	r_array_push(co->fp, r_array_pop(co->fp, rword) + 1, rword);
	rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));
	v = rvm_scope_tiplookup(co->scope, input, size);

	if (v->datatype == VARMAP_DATATYPE_FPOFFSET) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, DA, XX, v->data.offset));
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varalloc_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);
	if (v) {
		fprintf(stdout, "ERROR: variable already defined: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	r_array_push(co->fp, r_array_pop(co->fp, rword) + 1, rword);
	rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}




int codegen_newarraysize_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCARR, R0, R1, XX, 0));
//	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_newarraynosize_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCARR, R0, R1, XX, 0));
//	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_scopepush_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_scope_push(co->scope);
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_scopepop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_scope_pop(co->scope);
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


int codegen_funcallparameter_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_funcall_t *funcall = (rvm_funcall_t *)r_array_slot(co->funcall, r_array_length(co->funcall) - 1);

	funcall->params += 1;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_funcallexpression_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_funcall_t *funcall = r_array_empty(co->funcall) ? NULL : (rvm_funcall_t *) r_array_slot(co->funcall, r_array_length(co->funcall) - 1);


	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, funcall->params));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, DA, XX, funcall->params));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R1, FP, DA, -3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, DA, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TYPE, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, RVM_DTYPE_SWIID));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 5));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SWIID, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, SP, FP, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BL, R0, DA, XX, -rvm_codegen_getcodesize(co->cg)));

	r_array_remove(co->funcall);
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_funcallname_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_funcall_t funcall = {input, size, 0};

	r_array_add(co->funcall, &funcall);


	/*
	 * R0 holds the label of the called function, we save on the stack
	 * and FP will point there. After the call we save the LR at that spot
	 * as we don't need the RO anymore.
	 */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TYPE, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, RVM_DTYPE_POINTER));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R0, XX, 0));

	/*
	 * R1 is just a place holder, we will write the number of args passed
	 * later when we find out the number
	 */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R1)|BIT(FP)|BIT(SP)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_fundeclparameter_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_fundecl_t *fundecl = (rvm_fundecl_t *)r_array_lastslot(co->fundecl);
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);
	if (v) {
		fprintf(stdout, "ERROR: variable already defined: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	fundecl->params += 1;
	r_array_push(co->fp, r_array_pop(co->fp, rword) + 1, rword);
	rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_fundeclname_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_fundecl_t fundecl = {input, size, 0, off, 0};
	rint ret;

	ret = codegen_varalloc_callback(name, userdata, input, size, reason, start, end);
	if (ret == 0)
		return ret;

	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));

	r_array_push(co->fp, 0, rword);
	r_array_add(co->fundecl, &fundecl);
	rvm_scope_push(co->scope);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}



int codegen_fundeclsignature_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

//	rvm_codemap_add(cg->codemap, fundecl->funname, fundecl->funnamesiz, rvm_codegen_getcodesize(co->cg));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_fundecl_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_fundecl_t *fundecl = (rvm_fundecl_t *)r_array_lastslot(co->fundecl);
	rvm_varmap_t *fname;
	rword fp = r_array_pop(co->fp, rword);
	rvm_scope_pop(co->scope);
	fname = rvm_scope_tiplookup(co->scope, fundecl->funname, fundecl->funnamesiz);

	/*
	 * Function end, we first define the function end
	 */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xeeeeeeee));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(PC)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xeeeeeeee));

	fundecl->codesize = rvm_codegen_getcodesize(co->cg) - fundecl->codestart;


	if (fname->datatype == VARMAP_DATATYPE_FPOFFSET) {
		rvm_codegen_replaceins(co->cg, fundecl->codestart + 0, rvm_asm(RVM_ADDRS, R0, FP, DA, fname->data.offset));
	} else {
		rvm_codegen_replaceins(co->cg, fundecl->codestart + 0, rvm_asm(RVM_ADDRS, R0, DA, XX, fname->data.offset));
	}
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 1, rvm_asm(RVM_STRR, DA, R0, XX, fundecl->codestart + 3));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 2, rvm_asm(RVM_B, DA, XX, XX, fundecl->codesize - 2));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 3, rvm_asm(RVM_STS, LR, FP, DA, 0));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 4, rvm_asm(RVM_ADD, SP, FP, DA, fp));

	fundecl->codesize = rvm_codegen_getcodesize(co->cg) - fundecl->codestart;
	fundecl->params = r_array_last(co->fp, rword);

	r_array_remove(co->fundecl);
	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opreturn_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(FP)|BIT(SP)|BIT(PC)));

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifconditionop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {0, 0};

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	r_array_add(co->codespan, &cs);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, -1));	//This has to be redefined when we know the size of the code block

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.codestart + 1, rvm_asm(RVM_BEQ, DA, XX, XX, cs.codesize - 1));	//This has to be redefined when we know the size of the code block

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_elseop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, - 1)); 		// Branch to the end of the else block, has to be redefined
	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.codestart + 1, rvm_asm(RVM_BEQ, DA, XX, XX, cs.codesize - 1));	// Branch to the begining of the else block

	/* Reuse the cs to define the *else* codespan */
	cs.codestart = rvm_codegen_getcodesize(co->cg);
	cs.codesize = 0;
	r_array_add(co->codespan, &cs);

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifelseop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.codestart - 1, rvm_asm(RVM_B, DA, XX, XX, cs.codesize));	//Branch to the end of the else block, now we know the size

	codegen_print_callback(name, userdata, input, size, reason, start, end);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


void codegen_unmap_file(rstr_t *buf)
{
	if (buf) {
		munmap(buf->str, buf->size);
		r_free(buf);
	}
}


rstr_t *codegen_map_file(const char *filename)
{
	struct stat st;
	rstr_t *str;
	char *buffer;


	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return (void*)0;
	}
	if (fstat(fd, &st) < 0) {
		close(fd);
		return (void*)0;
	}
	buffer = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buffer == (void*)-1) {
		close(fd);
		return (void*)0;
	}
	str = (rstr_t *)r_malloc(sizeof(*str));
	if (!str)
		goto error;
	r_memset(str, 0, sizeof(*str));
	str->str = buffer;
	str->size = st.st_size;
	close(fd);
	return str;

error:
	munmap(buffer, st.st_size);
	close(fd);
	return str;
}


int main(int argc, char *argv[])
{
	int res, i;
	rvmcpu_t *cpu;
	ruint ntable;
	rpa_dbex_handle dbex = rpa_dbex_create();
	rvm_compiler_t *co = rvm_compiler_create(dbex);

	cpu = rvm_cpu_create();
	ntable = rvm_cpu_addswitable(cpu, switable);
	rvm_cpu_addswitable(cpu, switable_js);
	co->cpu = cpu;

	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		} else if (r_strcmp(argv[i], "-P") == 0) {
			parseinfo = 1;
			verboseinfo = 1;
		} else if (r_strcmp(argv[i], "-m") == 0) {
		}
	}

	rpagen_load_rules(dbex, co);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t script = { argv[i], r_strlen(argv[i]) };
				res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), script.str, script.str, script.str + script.size);
				if (res <= 0)
					rvm_codegen_clear(co->cg);
			}
			goto exec;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {
			rstr_t *script = NULL;
			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), script->str, script->str, script->str + script->size);
					codegen_unmap_file(script);
				}

			}
			goto exec;
		}
	}


exec:
	rvm_relocate(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));

	if (debuginfo) {
		fprintf(stdout, "\nGenerated Code:\n");
		rvm_asm_dump(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
		if (rvm_codegen_getcodesize(co->cg)) {
			fprintf(stdout, "\nExecution:\n");
			rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(co->cg, 0), 0);
		}
	} else {
		rvm_cpu_exec(cpu, rvm_codegen_getcode(co->cg, 0), 0);
	}


	rpa_dbex_destroy(dbex);
	rvm_cpu_destroy(cpu);
	rvm_compiler_destroy(co);

	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}


#define EOL "\n"


extern char _binary_____________tests_ecma262_rpa_start[];
extern char _binary_____________tests_ecma262_rpa_end[];
extern unsigned long *_binary_____________tests_ecma262_rpa_size;

void rpagen_load_rules(rpa_dbex_handle dbex, rvm_compiler_t *co)
{
	int ret, line;
	int inputsize = _binary_____________tests_ecma262_rpa_end - _binary_____________tests_ecma262_rpa_start;
	const char *buffer = _binary_____________tests_ecma262_rpa_start;
	const char *pattern = buffer;

	rpa_dbex_open(dbex);

	rpa_dbex_add_callback_exact(dbex, "BitwiseANDOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseXOROp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseOROp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AdditiveExpressionOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MultiplicativeExpressionOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ShiftExpressionOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "EqualityExpressionOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "RelationalExpressionOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalOROp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalANDOp", RPA_REASON_MATCHED, codegen_binary_asmop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "EqualityOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "RelationalOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AdditiveOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MultiplicativeOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ShiftOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseANDOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseXOROperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseOROperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalANDOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalOROperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "UnaryOperator", RPA_REASON_MATCHED, codegen_opcode_unary_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalNotOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseNotOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);


	rpa_dbex_add_callback_exact(dbex, "UnaryExpressionOp", RPA_REASON_MATCHED, codegen_unary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalNotExpressionOp", RPA_REASON_MATCHED, codegen_unary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseNotExpressionOp", RPA_REASON_MATCHED, codegen_unary_asmop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "DecimalIntegerLiteral", RPA_REASON_MATCHED, codegen_integer_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DecimalNonIntegerLiteral", RPA_REASON_MATCHED, codegen_double_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BlockBegin", RPA_REASON_MATCHED, codegen_scopepush_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BlockEnd", RPA_REASON_MATCHED, codegen_scopepop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PostfixExpressionInc", RPA_REASON_MATCHED, codegen_postfixinc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PostfixExpressionDec", RPA_REASON_MATCHED, codegen_postfixdec_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PrefixExpressionInc", RPA_REASON_MATCHED, codegen_prefixinc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PrefixExpressionDec", RPA_REASON_MATCHED, codegen_prefixdec_callback, co);

	rpa_dbex_add_callback_exact(dbex, "DoKeyword", RPA_REASON_MATCHED, codegen_dokeyword_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IterationDo", RPA_REASON_MATCHED, codegen_iterationdo_callback, co);



	rpa_dbex_add_callback_exact(dbex, "sqstring", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "dqstring", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DoubleStringCharacters", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "SingleStringCharacters", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ProgramInit", RPA_REASON_MATCHED, codegen_programinit_callback, co);
	rpa_dbex_add_callback_exact(dbex, "Program", RPA_REASON_MATCHED, codegen_program_callback, co);
	rpa_dbex_add_callback_exact(dbex, "Initialiser", RPA_REASON_MATCHED, codegen_opassign_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmentEq", RPA_REASON_MATCHED, codegen_opassign_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmentExpressionOp", RPA_REASON_MATCHED, codegen_opassign_callback, co);

	rpa_dbex_add_callback_exact(dbex, "VariableAllocate", RPA_REASON_MATCHED, codegen_varalloc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "VariableAllocateAndInit", RPA_REASON_MATCHED, codegen_varalloc_to_ptr_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ReturnOp", RPA_REASON_MATCHED, codegen_opreturn_callback, co);

	rpa_dbex_add_callback_exact(dbex, "SwiId", RPA_REASON_MATCHED, codegen_swiid_callback, co);
	rpa_dbex_add_callback_exact(dbex, "SwiIdExist", RPA_REASON_MATCHED, codegen_swiidexist_callback, co);

	rpa_dbex_add_callback_exact(dbex, "IdentifierOp", RPA_REASON_MATCHED, codegen_opidentifier_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PostfixExpression", RPA_REASON_MATCHED, codegen_postfixexpression_callback, co);


	rpa_dbex_add_callback_exact(dbex, "LeftHandSideExpressionPush", RPA_REASON_MATCHED, codegen_push_r0_callback, co);

	rpa_dbex_add_callback_exact(dbex, "MemberExpressionIndexBaseOp", RPA_REASON_MATCHED, codegen_memberexpressionbase_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MemberExpressionIndexOp", RPA_REASON_MATCHED, codegen_arrayelementaddress_callback, co);

	rpa_dbex_add_callback_exact(dbex, "CallExpressionIndexBaseOp", RPA_REASON_MATCHED, codegen_memberexpressionbase_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionIndexOp", RPA_REASON_MATCHED, codegen_arrayelementaddress_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ConditionalExpression", RPA_REASON_MATCHED, codegen_pop_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LeftHandSideExpressionDeref", RPA_REASON_MATCHED, codegen_ptr_deref_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IdentifierValue", RPA_REASON_MATCHED, codegen_identifiervalue_callback, co);


	rpa_dbex_add_callback_exact(dbex, "array_member_val", RPA_REASON_MATCHED, codegen_ptr_deref_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmetRightHandSide", RPA_REASON_MATCHED, codegen_pop_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "pop_r0", RPA_REASON_MATCHED, codegen_pop_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ValueOfExpression", RPA_REASON_MATCHED, codegen_pop_r0_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ArrayElementValue", RPA_REASON_MATCHED, codegen_arrayelementvalue_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ArrayElementAddress", RPA_REASON_MATCHED, codegen_arrayelementaddress_callback, co);
	rpa_dbex_add_callback_exact(dbex, "NewArraySize", RPA_REASON_MATCHED, codegen_newarraysize_callback, co);
	rpa_dbex_add_callback_exact(dbex, "NewArrayNoSize", RPA_REASON_MATCHED, codegen_newarraynosize_callback, co);
	rpa_dbex_add_callback_exact(dbex, "compile_error", RPA_REASON_MATCHED, codegen_compile_error_callback, co);

	rpa_dbex_add_callback_exact(dbex, "FunctionName", RPA_REASON_MATCHED, codegen_fundeclname_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionDefinition", RPA_REASON_MATCHED, codegen_fundeclsignature_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionDeclaration", RPA_REASON_MATCHED, codegen_fundecl_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionParameter", RPA_REASON_MATCHED, codegen_fundeclparameter_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionCallParameter", RPA_REASON_MATCHED, codegen_funcallparameter_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionCallName", RPA_REASON_MATCHED, codegen_funcallname_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionOp", RPA_REASON_MATCHED, codegen_funcallexpression_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IfConditionOp", RPA_REASON_MATCHED, codegen_ifconditionop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IfOp", RPA_REASON_MATCHED, codegen_ifop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ElseOp", RPA_REASON_MATCHED, codegen_elseop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IfElseOp", RPA_REASON_MATCHED, codegen_ifelseop_callback, co);


	if (verboseinfo)
		rpa_dbex_add_callback(dbex, ".*", RPA_REASON_MATCHED, codegen_print_callback, co);

	while ((ret = rpa_dbex_load(dbex, pattern, inputsize)) > 0) {
		inputsize -= ret;
		pattern += ret;
	}
	if (ret < 0) {
		for (line = 1; pattern >= buffer; --pattern) {
			if (*pattern == '\n')
				line += 1;
		}
		fprintf(stdout, "Line: %d, RPA LOAD ERROR: %s\n", line, (rpa_dbex_get_error(dbex) == RPA_E_SYNTAX_ERROR) ? "Syntax Error." : "Pattern Loading failed.");
		goto error;
	}

error:
	rpa_dbex_close(dbex);
}
