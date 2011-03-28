#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rjsobject.h"
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
static int compileonly = 0;

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


typedef enum {
	RVM_CODESPAN_NONE = 0,
	RVM_CODESPAN_LOOP,
	RVM_CODESPAN_SWITCH,
} rvm_codespantype_t;

typedef struct rvm_codespan_s {
	rvm_codespantype_t type;
	rword codestart;
	rword codesize;
	rword l1;
	rword l2;
	rword l3;
	rword l4;
} rvm_codespan_t;


typedef struct rvm_costat_s rvm_costat_t;

struct rvm_costat_s {
	ruint binaryop;
	ruint level;
	ruint dirty;
	ruint funcalls;
	rvm_costat_t *root;
};

typedef struct rvm_compiler_s {
	rpa_dbex_handle dbex;
	rvm_codegen_t *cg;
	rvm_scope_t *scope;
	rarray_t *fp;
	rarray_t *funcall;
	rarray_t *fundecl;
	rarray_t *opcodes;
	rarray_t *codespan;
	rarray_t *loops;
	rarray_t *varmap;
	rarray_t *stat;
	rvmcpu_t *cpu;
	rboolean optimized;
} rvm_compiler_t;



#define RVM_SAVEDREGS_FIRST R4
#define RVM_SAVEDREGS_MAX (RLST - (RLST - R8) - RVM_SAVEDREGS_FIRST)


void rpagen_load_rules(rpa_dbex_handle dbex, rvm_compiler_t *co);

rvm_costat_t *rvm_costat_current(rvm_compiler_t *co)
{
	return (rvm_costat_t *)r_array_lastslot(co->stat);
}


ruint rvm_costat_getdebth(rvm_compiler_t *co)
{
	return r_array_length(co->stat);
}

ruint rvm_costat_getlevel(rvm_compiler_t *co)
{
	if (!rvm_costat_current(co))
		return 0;
	return rvm_costat_current(co)->level;
}


void rvm_costat_inclevel(rvm_compiler_t *co)
{
	rvm_costat_current(co)->level += 1;
	if (rvm_costat_current(co)->level > rvm_costat_current(co)->root->dirty)
		rvm_costat_current(co)->root->dirty = rvm_costat_current(co)->level;
}

void rvm_costat_declevel(rvm_compiler_t *co)
{
	rvm_costat_current(co)->level -= 1;
}


ruint rvm_costat_getbinaryop(rvm_compiler_t *co)
{
	if (!rvm_costat_current(co))
		return 0;
	return rvm_costat_current(co)->binaryop;
}


void rvm_costat_incbinaryop(rvm_compiler_t *co)
{
	rvm_costat_current(co)->binaryop += 1;
}


void rvm_costat_decbinaryop(rvm_compiler_t *co)
{
	rvm_costat_current(co)->binaryop -= 1;
}

ruint rvm_costat_getdirty(rvm_compiler_t *co)
{
	if (!rvm_costat_current(co))
		return 0;
	return rvm_costat_current(co)->root->dirty;
}


void rvm_costat_push(rvm_compiler_t *co)
{
	rvm_costat_t stat = *rvm_costat_current(co);
	stat.binaryop = 0;
	r_array_add(co->stat, &stat);
}


void rvm_costat_pushroot(rvm_compiler_t *co)
{
	rvm_costat_t stat;
	r_memset(&stat, 0, sizeof(stat));
	r_array_add(co->stat, &stat);
	rvm_costat_current(co)->root = rvm_costat_current(co);
}

void rvm_costat_pop(rvm_compiler_t *co)
{
	r_array_removelast(co->stat);
}



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
	co->stat = r_array_create(sizeof(rvm_costat_t));
	co->fundecl = r_array_create(sizeof(rvm_fundecl_t));
	co->codespan = r_array_create(sizeof(rvm_codespan_t));
	co->loops = r_array_create(sizeof(rvm_codespan_t*));
	co->varmap = r_array_create(sizeof(rvm_varmap_t*));
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
		r_object_destroy((robject_t*) co->stat);
		r_object_destroy((robject_t*) co->fp);
		r_object_destroy((robject_t*) co->funcall);
		r_object_destroy((robject_t*) co->fundecl);
		r_object_destroy((robject_t*) co->codespan);
		r_object_destroy((robject_t*) co->varmap);
		r_object_destroy((robject_t*) co->loops);
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
	rvmreg_t *arg1 = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 1);
	rvmreg_t *arg2 = (rvmreg_t *)RVM_STACK_ADDR(cpu->stack, RVM_CPUREG_GETU(cpu, FP) + 2);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_ADD, cpu, res, arg1, arg2);
}


static void rvm_swi_object(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *res = RVM_CPUREG_PTR(cpu, R0);
	rharray_t *a = r_harray_create_rvmreg();
	rvm_reg_setharray(res, (robject_t*)a);
}


static rvm_switable_t switable[] = {
		{"none", test_swi_none},
		{"RVM_SWI_NEG", rvm_swi_negative},
		{"Object", rvm_swi_object},
		{"rvm_swi_eadd", rvm_swi_eadd},
		{NULL, NULL},
};


static void rvm_js_print(rvmcpu_t *cpu, rvm_asmins_t *ins)
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

static rvm_switable_t switable_js[] = {
		{"print", rvm_js_print},
		{NULL, NULL},
};

inline int codegen_print_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	const rpa_cbrecord_t *rec;


	if (parseinfo) {
		if (reason & RPA_REASON_START)
			fprintf(stdout, "START ");
		if (reason & RPA_REASON_MATCHED)
			fprintf(stdout, "MATCHED ");
		if (reason & RPA_REASON_END)
			fprintf(stdout, "END ");
		fprintf(stdout, "%s: ", name);
		if (!(reason & RPA_REASON_START))
				fwrite(input, sizeof(char), size, stdout);
		rec = rpa_stat_cbrecord_lookahead(stat, rpa_stat_cbrecord_current(stat), NULL, input+size, RPA_REASON_MATCHED, 0);
		if (rec) {
			fprintf(stdout, " (next: %s, input: %p)", rec->name, rec->input);
		}
		fprintf(stdout, " (debth: %d, level: %d, binaryop: %d, dirty: %d)", rvm_costat_getdebth(co), rvm_costat_getlevel(co), rvm_costat_getbinaryop(co), rvm_costat_getdirty(co));
//		fprintf(stdout, "(RVM_SAVEDREGS_MAX = %d)", RVM_SAVEDREGS_MAX);
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


int codegen_opcode_unary_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;

	if (r_stringncmp("-", input, size))
		r_array_push(co->opcodes, RVM_OPSWI(rvm_cpu_getswi_s(co->cpu, "RVM_SWI_NEG")), ruint);
	else if (r_stringncmp("+", input, size))
		r_array_push(co->opcodes, RVM_NOP, ruint);
	else if (r_stringncmp("!", input, size))
		r_array_push(co->opcodes, RVM_ELNOT, ruint);
	else if (r_stringncmp("~", input, size))
		r_array_push(co->opcodes, RVM_ENOT, ruint);
	else if (r_stringncmp("delete", input, size))
		r_array_push(co->opcodes, RVM_NOP, ruint);
	else if (r_stringncmp("void", input, size))
		r_array_push(co->opcodes, RVM_NOP, ruint);
	else if (r_stringncmp("typeof", input, size))
		r_array_push(co->opcodes, RVM_NOP, ruint);
	else
		r_array_push(co->opcodes, RVM_ABORT, ruint);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	return size;
}


int codegen_opcode_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;

	if (r_stringncmp("++", input,  size))
		r_array_push(co->opcodes, RVM_EADD, ruint);
	else if (r_stringncmp("+", input,  size))
		r_array_push(co->opcodes, RVM_EADD, ruint);
	else if (r_stringncmp("+=", input,  size))
		r_array_push(co->opcodes, RVM_EADD, ruint);
	else if (r_stringncmp("--", input,  size))
		r_array_push(co->opcodes, RVM_ESUB, ruint);
	else if (r_stringncmp("-", input,  size))
		r_array_push(co->opcodes, RVM_ESUB, ruint);
	else if (r_stringncmp("-=", input,  size))
		r_array_push(co->opcodes, RVM_ESUB, ruint);
	else if (r_stringncmp("*", input,  size))
		r_array_push(co->opcodes, RVM_EMUL, ruint);
	else if (r_stringncmp("*=", input,  size))
		r_array_push(co->opcodes, RVM_EMUL, ruint);
	else if (r_stringncmp("/", input,  size))
		r_array_push(co->opcodes, RVM_EDIV, ruint);
	else if (r_stringncmp("/=", input,  size))
		r_array_push(co->opcodes, RVM_EDIV, ruint);
	else if (r_stringncmp("%", input,  size))
		r_array_push(co->opcodes, RVM_EMOD, ruint);
	else if (r_stringncmp("%=", input,  size))
		r_array_push(co->opcodes, RVM_EMOD, ruint);
	else if (r_stringncmp("&&", input,  size))
		r_array_push(co->opcodes, RVM_ELAND, ruint);
	else if (r_stringncmp("||", input,  size))
		r_array_push(co->opcodes, RVM_ELOR, ruint);
	else if (r_stringncmp("&", input,  size))
		r_array_push(co->opcodes, RVM_EAND, ruint);
	else if (r_stringncmp("&=", input,  size))
		r_array_push(co->opcodes, RVM_EAND, ruint);
	else if (r_stringncmp("|", input,  size))
		r_array_push(co->opcodes, RVM_EORR, ruint);
	else if (r_stringncmp("|=", input,  size))
		r_array_push(co->opcodes, RVM_EORR, ruint);
	else if (r_stringncmp("^", input,  size))
		r_array_push(co->opcodes, RVM_EXOR, ruint);
	else if (r_stringncmp("^=", input,  size))
		r_array_push(co->opcodes, RVM_EXOR, ruint);
	else if (r_stringncmp(">>", input,  size))
		r_array_push(co->opcodes, RVM_ELSR, ruint);
	else if (r_stringncmp(">>=", input,  size))
		r_array_push(co->opcodes, RVM_ELSR, ruint);
	else if (r_stringncmp("<<", input,  size))
		r_array_push(co->opcodes, RVM_ELSL, ruint);
	else if (r_stringncmp("<<=", input,  size))
		r_array_push(co->opcodes, RVM_ELSL, ruint);
	else if (r_stringncmp(">>>", input,  size))
		r_array_push(co->opcodes, RVM_ELSRU, ruint);
	else if (r_stringncmp(">>>=", input,  size))
		r_array_push(co->opcodes, RVM_ELSRU, ruint);
	else if (r_stringncmp("<=", input,  size))
		r_array_push(co->opcodes, RVM_ELESSEQ, ruint);
	else if (r_stringncmp(">=", input,  size))
		r_array_push(co->opcodes, RVM_EGREATEQ, ruint);
	else if (r_stringncmp("<", input,  size))
		r_array_push(co->opcodes, RVM_ELESS, ruint);
	else if (r_stringncmp(">", input,  size))
		r_array_push(co->opcodes, RVM_EGREAT, ruint);
	else if (r_stringncmp("===", input,  size))
		r_array_push(co->opcodes, RVM_EEQ, ruint);
	else if (r_stringncmp("==", input,  size))
		r_array_push(co->opcodes, RVM_EEQ, ruint);
	else if (r_stringncmp("!==", input,  size))
		r_array_push(co->opcodes, RVM_ENOTEQ, ruint);
	else if (r_stringncmp("!=", input,  size))
		r_array_push(co->opcodes, RVM_ENOTEQ, ruint);
	else if (r_stringncmp("!", input,  size))
		r_array_push(co->opcodes, RVM_ELNOT, ruint);
	else if (r_stringncmp("~", input,  size))
		r_array_push(co->opcodes, RVM_ENOT, ruint);
	else if (r_stringncmp("=", input,  size))
		r_array_push(co->opcodes, RVM_NOP, ruint);
	else
		r_array_push(co->opcodes, RVM_ABORT, ruint);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	return size;
}


int codegen_binary_asmop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_costat_inclevel(co);
		rvm_costat_incbinaryop(co);
	} else if (reason & RPA_REASON_MATCHED) {
		ruint op1;
		ruint opcode = r_array_pop(co->opcodes, ruint);

		if (rvm_costat_getlevel(co) < RVM_SAVEDREGS_MAX) {
			op1 = rvm_costat_getlevel(co) + RVM_SAVEDREGS_FIRST - 1;
		} else {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
			op1 = R1;
		}
		rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, op1, R0, 0));
		rvm_costat_declevel(co);
		rvm_costat_decbinaryop(co);
	} else if (reason & RPA_REASON_END) {
		rvm_costat_declevel(co);
		rvm_costat_decbinaryop(co);
	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_valop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_asmins_t *last = rvm_codegen_getcode(co->cg, off - 1);

	if ((reason & RPA_REASON_START) && rvm_costat_getbinaryop(co)) {
		if (rvm_costat_getlevel(co) < RVM_SAVEDREGS_MAX) {
			if (co->optimized && last->op1 == R0 && (last->opcode == RVM_MOV || last->opcode == RVM_LDRR ||
					last->opcode == RVM_LDS || (last->opcode >= RVM_EADD && last->opcode <= RVM_ELESSEQ))) {
				last->op1 = rvm_costat_getlevel(co) + RVM_SAVEDREGS_FIRST - 1;
				off -= 1;
			} else {
				rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, rvm_costat_getlevel(co) + RVM_SAVEDREGS_FIRST - 1, R0, XX, 0));
			}
		} else {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
		}
	}


	if (reason & RPA_REASON_MATCHED) {

	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_conditionalexp_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_unary_asmop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint op1 = rvm_costat_getlevel(co) ? RVM_SAVEDREGS_FIRST - 1 + rvm_costat_getlevel(co) : R0;

	if (reason & RPA_REASON_START) {
	}

	if (reason & RPA_REASON_MATCHED) {
		ruint opcode = r_array_pop(co->opcodes, ruint);
		rvm_codegen_addins(co->cg, rvm_asm(opcode, op1, op1, XX, 0));
	}

	if (reason & RPA_REASON_END) {
	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_costate_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_costat_push(co);
	}

	if (reason & RPA_REASON_END) {
		rvm_costat_pop(co);
	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_push_r0_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_integer_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R0, DA, XX, r_strtol(input, NULL, 10)));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_printop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmd(RVM_PRN, R0, DA, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_double_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmd(RVM_MOV, R0, DA, XX, r_strtod(input, NULL)));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_string_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, (void*)input));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R2, DA, XX, size));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCSTR, R0, R1, R2, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_program_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	ruint off;

	if (reason & RPA_REASON_START) {
		off = rvm_codegen_getcodesize(co->cg);
		rvm_costat_pushroot(co);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0));
//		rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCOBJ, R8, DA, XX, 0));
//		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, TP, R8, XX, 0));
	} else {
		rvm_codegen_replaceins(co->cg, 0, rvm_asm(RVM_MOV, FP, SP, XX, 0));
		rvm_codegen_replaceins(co->cg, 1, rvm_asm(RVM_ADD, SP, SP, DA, r_array_pop(co->fp, rword)));
		off = rvm_codegen_getcodesize(co->cg);
		if (debuginfo)
			rvm_codegen_addins(co->cg, rvm_asm(RVM_PRN, R0, XX, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	}


	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	if (reason & RPA_REASON_END) {
		rvm_costat_pop(co);
	}
	return size;
}


int codegen_opinit_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opassign_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint opcode = r_array_pop(co->opcodes, ruint);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	if (opcode != RVM_NOP) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R2, R1, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R2, R0, 0));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_oppostfix_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint opcode = r_array_pop(co->opcodes, ruint);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R2, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R2, R1, XX, 0));


	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opprefix_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	ruint opcode = r_array_pop(co->opcodes, ruint);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDRR, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(opcode, R0, R0, DA, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_addressoflefthandside_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_asmins_t *last = rvm_codegen_getcode(co->cg, off - 1);
	rvm_asmins_t *lastlast = rvm_codegen_getcode(co->cg, off - 2);

	if (last->opcode == RVM_LDRR) {
		last->opcode = RVM_MOV;
		off -= 1;
	} else if (last->opcode == RVM_LDS) {
		last->opcode = RVM_ADDRS;
		off -= 1;
	} else if (last->opcode == RVM_LDOBJN) {
		last->opcode = RVM_ADDROBJN;
		off -= 1;
	} else if (last->opcode == RVM_LDOBJH && lastlast->opcode == RVM_OBJLKUP) {
		lastlast->opcode = RVM_OBJLKUPADD;
		last->opcode = RVM_ADDROBJH;
		off -= 2;
	} else {
		fprintf(stdout, "ERROR: Invalid Left-Hand side expression: ");
		fwrite(input, sizeof(char), size, stdout);
		fprintf(stdout, "\n");
		rpa_dbex_abort(co->dbex);
		return 0;
	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}



int codegen_n_arrayelementvalue_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Array
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDOBJN, R0, R1, R0, 0));	// Load the value from array offset

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_h_arraynamelookup_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 	// Supposedly Array Address
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, size));
	rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R2, DA, XX, (void*)input));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_OBJLKUP, R0, R1, R2, 0));	// Get the address of the element at offset R0

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_h_arrayelementvalue_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDOBJH, R0, R1, R0, 0));	// Get the address of the element at offset R0

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_swiidexist_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rint swi = rvm_cpu_getswi(co->cpu, input, size);

	if (swi < 0)
		return 0;


	return size;
}


int codegen_swiid_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rint swi = rvm_cpu_getswi(co->cpu, input, size);

	if (swi < 0)
		return 0;

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, swi));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_SWIID));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opthis_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, TP, XX, 0));
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;

}


int codegen_opidentifier_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_varmap_t *v = rvm_scope_lookup(co->scope, input, size);

	if (v) {
		if (v->datatype == VARMAP_DATATYPE_OFFSET) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, FP, DA, v->data.offset));
		} else {
			rvm_codegen_addins(co->cg, rvm_asmp(RVM_LDRR, R0, DA, XX, v->data.ptr));
		}
		codegen_print_callback(stat, name, userdata, input, size, reason);
		codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

		return size;
	}

	fprintf(stdout, "ERROR: undefined variable: ");
	fwrite(input, sizeof(char), size, stdout);
	fprintf(stdout, "\n");
	rpa_dbex_abort(co->dbex);
	return 0;
}


int codegen_varalloc_to_ptr_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
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

	if (rvm_scope_count(co->scope)) {
		r_array_inclast(co->fp, rword);
		rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));
	} else {
		rvm_scope_addpointer(co->scope, input, size, r_carray_slot_expand(co->cpu->data, r_carray_length(co->cpu->data)));
		r_carray_inclength(co->cpu->data);
	}
	v = rvm_scope_tiplookup(co->scope, input, size);

	if (v->datatype == VARMAP_DATATYPE_OFFSET) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R0, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R0, DA, XX, v->data.ptr));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_varalloc_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
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

	if (rvm_scope_count(co->scope)) {
		r_array_inclast(co->fp, rword);
		rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));
	} else {
		rvm_scope_addpointer(co->scope, input, size, r_carray_slot_expand(co->cpu->data, r_carray_length(co->cpu->data)));
		r_carray_inclength(co->cpu->data);
	}

	v = rvm_scope_tiplookup(co->scope, input, size);
	if (v->datatype == VARMAP_DATATYPE_OFFSET) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R1, FP, DA, v->data.offset));
	} else {
		rvm_codegen_addins(co->cg, rvm_asmp(RVM_MOV, R1, DA, XX, v->data.ptr));
	}
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLRR, R1, XX, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opnewarray_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCOBJ, R0, DA, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_scopepush_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_scope_push(co->scope);
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_scopepop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	rvm_scope_pop(co->scope);
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_compile_error_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	fprintf(stdout, "COMPILE ERROR pos: %ld\n", (long) (input - rpa_stat_input_start(stat)));
	rpa_dbex_abort(co->dbex);
	return 0;
}


int codegen_funcallparameter_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_funcall_t *funcall = (rvm_funcall_t *)r_array_slot(co->funcall, r_array_length(co->funcall) - 1);

	funcall->params += 1;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_newkeyword_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);


	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_newexpressioncallop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_funcall_t funcall = {input, size, 0};
		r_array_add(co->funcall, &funcall);

		rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(TP)|BIT(FP)|BIT(SP)|BIT(LR)));
	} else if (reason & RPA_REASON_MATCHED){
		rvm_funcall_t *funcall = r_array_empty(co->funcall) ? NULL : (rvm_funcall_t *) r_array_slot(co->funcall, r_array_length(co->funcall) - 1);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_ALLOCOBJ, TP, DA, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, funcall->params));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CALL, R0, DA, XX, -rvm_codegen_getcodesize(co->cg)));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, TP, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(TP,LR)));
		r_array_removelast(co->funcall);
	} else if (reason & RPA_REASON_END) {
		/*
		 * We should never get here
		 */
		r_array_removelast(co->funcall);
		R_ASSERT(0);
	}


	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_funcallname_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_asmins_t *last = rvm_codegen_getcode(co->cg, off - 1);

	if (last->op1 == R0 && last->op2 == R1 && (last->opcode == RVM_LDOBJN || last->opcode == RVM_LDOBJH)) {
		/*
		 * The function call id is comming from an object, so we set the this pointer (TP)
		 * to point to the object (R1)
		 */
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, TP, R1, XX, 0));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, TP, R8, XX, 0));
	}
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_funcallexpression_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_funcall_t funcall = {input, size, 0};

		r_array_add(co->funcall, &funcall);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(TP)|BIT(FP)|BIT(SP)|BIT(LR)));
	} else if (reason & RPA_REASON_MATCHED) {
		rvm_funcall_t *funcall = r_array_empty(co->funcall) ? NULL : (rvm_funcall_t *) r_array_slot(co->funcall, r_array_length(co->funcall) - 1);

		rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, FP, SP, DA, funcall->params));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CALL, R0, XX, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(TP,LR)));
		r_array_removelast(co->funcall);
	} else if (reason & RPA_REASON_END) {
		/*
		 * We should never get here
		 */
		r_array_removelast(co->funcall);
		R_ASSERT(0);
	}
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_fundeclparameter_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
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
	r_array_inclast(co->fp, rword);
	rvm_scope_addoffset(co->scope, input, size, r_array_last(co->fp, rword));
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_funnamelookupalloc_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);

	if (!v) {
		if (codegen_varalloc_callback(stat, name, userdata, input, size, reason) == 0)
			return 0;
	}
	return size;
}


int codegen_fundeclname_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off;
	rvm_fundecl_t fundecl = {input, size, 0, 0};
	rvm_varmap_t *v = rvm_scope_tiplookup(co->scope, input, size);

	if (!v) {
		if (codegen_varalloc_callback(stat, name, userdata, input, size, reason) == 0)
			return 0;
	}
	off = rvm_codegen_getcodesize(co->cg);
	fundecl.codestart = off;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, DA, XX, 0)); 	/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 0)); 	/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION)); 	/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0)); 	/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0)); 		/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, SP, FP, DA, 0)); 	/* Will be re-written later */
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xffffffff));

	r_array_push(co->fp, 0, rword);
	r_array_add(co->fundecl, &fundecl);
	rvm_scope_push(co->scope);
	rvm_costat_pushroot(co);
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_fundecl_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
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
	if (rvm_costat_getdirty(co))
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(RVM_SAVEDREGS_FIRST, RVM_SAVEDREGS_FIRST+ rvm_costat_getdirty(co) - 1)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	/*
	 * the number of arguments passed is = SP - FP
	 */
	fundecl->codesize = rvm_codegen_getcodesize(co->cg) - fundecl->codestart;
	if (fname->datatype == VARMAP_DATATYPE_OFFSET) {
		rvm_codegen_replaceins(co->cg, fundecl->codestart + 0, rvm_asm(RVM_ADDRS, R1, FP, DA, fname->data.offset));
	} else {
		rvm_codegen_replaceins(co->cg, fundecl->codestart + 0, rvm_asmp(RVM_MOV, R1, DA, XX, fname->data.ptr));
	}
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 1, rvm_asm(RVM_ADD, R0, PC, DA, sizeof(rvm_asmins_t) * 3));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 2, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 3, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 4, rvm_asm(RVM_B, DA, XX, XX, fundecl->codesize - 4));
	rvm_codegen_replaceins(co->cg, fundecl->codestart + 5, rvm_asm(RVM_ADD, SP, FP, DA, fp));
	if (rvm_costat_getdirty(co))
		rvm_codegen_replaceins(co->cg, fundecl->codestart + 6, rvm_asm(RVM_PUSHM, DA, XX, XX, BITS(RVM_SAVEDREGS_FIRST, RVM_SAVEDREGS_FIRST + rvm_costat_getdirty(co) - 1)));

	fundecl->codesize = rvm_codegen_getcodesize(co->cg) - fundecl->codestart;
	fundecl->params = r_array_last(co->fp, rword);

	r_array_removelast(co->fundecl);
	rvm_costat_pop(co);
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_opreturn_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (rvm_costat_getdirty(co))
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(RVM_SAVEDREGS_FIRST, RVM_SAVEDREGS_FIRST + rvm_costat_getdirty(co) - 1)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifconditionop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	r_array_add(co->codespan, &cs);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, -1));	//This has to be redefined when we know the size of the code block

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.codestart + 1, rvm_asm(RVM_BEQ, DA, XX, XX, cs.codesize - 1));	//This has to be redefined when we know the size of the code block

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_elseop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
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

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_ifelseop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.codestart - 1, rvm_asm(RVM_B, DA, XX, XX, cs.codesize));	//Branch to the end of the else block, now we know the size

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_dokeyword_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	r_array_add(co->codespan, &cs);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_iterationdo_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
	rvm_asmins_t *last = rvm_codegen_getcode(co->cg, off - 1);

	if (last->op1 == R0 && last->opcode >= RVM_EADD && last->opcode <= RVM_ELESSEQ) {
		/*
		 * Nothing to do, the status register will be updated.
		 */
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	}

	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, -cs.codesize));

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_whileconditionop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t*)r_array_lastslot(co->codespan);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	cs->l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, -1));	//This has to be redefined when we know the size of the code block

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_iterationwhileop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};

		cs.codestart = rvm_codegen_getcodesize(co->cg);
		r_array_add(co->codespan, &cs);
	} else if (reason & RPA_REASON_MATCHED) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

		cs.codesize = rvm_codegen_getcodesize(co->cg) + 1 - cs.codestart;
		rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, -(cs.codesize - 1)));
		rvm_codegen_replaceins(co->cg, cs.l1, rvm_asm(RVM_BEQ, DA, XX, XX, cs.codesize - (cs.l1 - cs.codestart)));	// Re-writing the instruction
	} else if (reason & RPA_REASON_END) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
		cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_questionmarkop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	cs.l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 0)); // This will be re-written
	r_array_add(co->codespan, &cs);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_iftrueop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t*) r_array_lastslot(co->codespan);

	cs->l2 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0)); // This will be re-written
	rvm_codegen_replaceins(co->cg, cs->l1, rvm_asm(RVM_BEQ, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs->l1));	// Re-writing the instruction

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_iffalseop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	rvm_codegen_replaceins(co->cg, cs.l2, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs.l2));	// Re-writing the instruction

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);

	return size;
}


int codegen_breakkeyword_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = NULL;

	if (r_array_empty(co->loops)) {
		fprintf(stdout, "ERROR: Invalid break use. ");
		fprintf(stdout, "\n");
		return 0;
	}
	cs = (rvm_codespan_t *)r_array_last(co->loops, rvm_codespan_t*);

	if (cs->type == RVM_CODESPAN_LOOP || cs->type == RVM_CODESPAN_SWITCH) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, -(rvm_codegen_getcodesize(co->cg) - cs->l2)));
	} else {
		/*
		 * Error
		 */

	}

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_continuekeyword_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_last(co->loops, rvm_codespan_t*);

	if (cs->type != RVM_CODESPAN_LOOP) {
		return 0;
	}

	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, -(rvm_codegen_getcodesize(co->cg) - cs->l3)));


	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_forkeyword_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {RVM_CODESPAN_LOOP, 0, 0, 0, 0, 0, 0};

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	r_array_add(co->codespan, &cs);
	rvm_scope_push(co->scope);
	r_array_push(co->loops, r_array_lastslot(co->codespan), rvm_codespan_t*);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_forinitop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_lastslot(co->codespan);


	cs->l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 1)); // This will be re-written, when we know the start point of the comparison expression
	cs->l2 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 1)); // Break point, This will be re-written.
	cs->l3 = rvm_codegen_getcodesize(co->cg);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}

/*
 * The code generated for the increment expression will appear before the comparison expression
 * due to the trick we did in the BNF schema. That will make the implementation of the for ( ; ; ) loops
 * a lot more easier and the generated code having less branch instructions
 */
int codegen_forincrementop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_lastslot(co->codespan);

	rvm_codegen_replaceins(co->cg, cs->l1, rvm_asm(RVM_B, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs->l1));	// Re-writing the instruction

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_forcompareop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_lastslot(co->codespan);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	cs->l4 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 0)); // This will be re-written when we know the size of the code inside the loop.
	/*
	 * The beginning loop starts here, right after the RVM_BEQ instruction.
	 */

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_iterationforop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, -(rvm_codegen_getcodesize(co->cg) - (cs.l3))));
	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	rvm_codegen_replaceins(co->cg, cs.l4, rvm_asm(RVM_BEQ, DA, XX, XX, cs.codesize - (cs.l4 - cs.codestart)));
	rvm_codegen_replaceins(co->cg, cs.l2, rvm_asm(RVM_B, DA, XX, XX, cs.codesize - (cs.l2 - cs.codestart)));	// Re-writing the instruction

	rvm_scope_pop(co->scope);
	r_array_removelast(co->loops);

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_switchexpressionop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_lastslot(co->codespan);

	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 2));
	cs->l2 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0)); /* Re-write later, this is the break point */

	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


/*
 * l1: the default statement
 * l2: the break point
 *
 */

int codegen_switchstatementop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_codespan_t cs = {RVM_CODESPAN_SWITCH, 0, 0, 0, 0, 0, 0};
		cs.codestart = rvm_codegen_getcodesize(co->cg);
		r_array_add(co->codespan, &cs);
		r_array_push(co->loops, r_array_lastslot(co->codespan), rvm_codespan_t*);
	} else if (reason & RPA_REASON_MATCHED) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
		r_array_removelast(co->loops);

		if (cs.l1 > 0) {
			rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 1));
			rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R0, SP, DA, -1));
			rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, cs.l1 - rvm_codegen_getcodesize(co->cg)));
		}

		cs.l4 = rvm_codegen_getcodesize(co->cg);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R0, XX, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R2, XX, XX, 0));
		cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
		rvm_codegen_replaceins(co->cg, cs.l2, rvm_asm(RVM_B, DA, XX, XX, cs.l4 - cs.l2));	// Re-writing the instruction
	} else if (reason & RPA_REASON_END) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
		r_array_removelast(co->loops);
		cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	}
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_caseexpressionop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t *cs = (rvm_codespan_t *)r_array_lastslot(co->codespan);

	if (reason & RPA_REASON_START) {
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R1, SP, DA, -1));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, 0));
		cs->l1 = rvm_codegen_getcodesize(co->cg);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 0));



	} else if (reason & RPA_REASON_MATCHED) {

		/*
		 * Evaluate the expression, the result is in R0
		 */
		rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R1, SP, XX, 0));
		/*
		 * Compare the expression with the switch(expression)
		 */
		rvm_codegen_addins(co->cg, rvm_asm(RVM_EEQ, R0, R0, R1, 0));

		rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
		cs->l2 = rvm_codegen_getcodesize(co->cg);
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 0));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_STS, R0, SP, DA, -1));

		/*
		 * Rewrite the branch instruction in the beginning of the case caluse
		 */
		rvm_codegen_replaceins(co->cg, cs->l1, rvm_asm(RVM_BNEQ, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs->l1));	// Re-writing the instruction
		rvm_codegen_replaceins(co->cg, cs->l2, rvm_asm(RVM_BEQ, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs->l2));	// Re-writing the instruction

	} else if (reason & RPA_REASON_END) {

	}
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_caseclauseop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);

	if (reason & RPA_REASON_START) {
		rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};
		cs.codestart = rvm_codegen_getcodesize(co->cg);
		r_array_add(co->codespan, &cs);
	} else if (reason & RPA_REASON_MATCHED) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
		cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
		cs.l3 = rvm_codegen_getcodesize(co->cg);
		rvm_codegen_replaceins(co->cg, cs.l2, rvm_asm(RVM_BEQ, DA, XX, XX, cs.l3 - cs.l2));
	} else if (reason & RPA_REASON_END) {
		rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
		cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	}
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_defaultkeywordop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = {RVM_CODESPAN_NONE, 0, 0, 0, 0, 0, 0};
	rvm_codespan_t *csSwitch = (rvm_codespan_t *)r_array_last(co->loops, rvm_codespan_t*);

	if (csSwitch->type == RVM_CODESPAN_SWITCH) {
		if (csSwitch->l1 > 0) {
			fprintf(stdout, "ERROR: More than one switch default: %ld\n", csSwitch->l1);
			rpa_dbex_abort(co->dbex);
			return 0;
		}
		csSwitch->l1 = rvm_codegen_getcodesize(co->cg);
	}

	cs.codestart = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R1, SP, DA, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R1, DA, XX, 0));
	cs.l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 0));
	r_array_add(co->codespan, &cs);
	codegen_print_callback(stat, name, userdata, input, size, reason);
	codegen_dump_code(rvm_codegen_getcode(co->cg, off), rvm_codegen_getcodesize(co->cg) - off);
	return size;
}


int codegen_defaultclauseop_callback(rpa_stat_handle stat, const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason)
{
	rvm_compiler_t *co = (rvm_compiler_t *)userdata;
	rulong off = rvm_codegen_getcodesize(co->cg);
	rvm_codespan_t cs = r_array_pop(co->codespan, rvm_codespan_t);
	cs.codesize = rvm_codegen_getcodesize(co->cg) - cs.codestart;
	cs.l3 = rvm_codegen_getcodesize(co->cg);

	rvm_codegen_replaceins(co->cg, cs.l1, rvm_asm(RVM_BEQ, DA, XX, XX, rvm_codegen_getcodesize(co->cg) - cs.l1));	// Re-writing the instruction
	codegen_print_callback(stat, name, userdata, input, size, reason);
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
	rstr_t *script = NULL, *unmapscript = NULL;
	rvmcpu_t *cpu;
	rvmreg_t *thisptr;
	ruint ntable;
	rpa_dbex_handle dbex = rpa_dbex_create();
	rvm_compiler_t *co = rvm_compiler_create(dbex);

	cpu = rvm_cpu_create_default();
	ntable = rvm_cpu_addswitable(cpu, switable);
	rvm_cpu_addswitable(cpu, switable_js);
	co->cpu = cpu;

	thisptr = rvm_cpu_alloc_global(cpu);
	rvm_reg_setjsobject(thisptr, (robject_t *)rjs_object_create(sizeof(rvmreg_t)));
	rvm_cpu_setreg(cpu, R8, thisptr);
	rvm_cpu_setreg(cpu, TP, thisptr);


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
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-o") == 0) {
			co->optimized = 1;
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

			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					res = rpa_dbex_parse(dbex, rpa_dbex_default_pattern(dbex), script->str, script->str, script->str + script->size);
					unmapscript = script;
					if (res <= 0)
						rvm_codegen_clear(co->cg);
				}

			}
			goto exec;
		}
	}


exec:
	if (debuginfo) {
		fprintf(stdout, "\nGenerated Code:\n");
		rvm_asm_dump(rvm_codegen_getcode(co->cg, 0), rvm_codegen_getcodesize(co->cg));
		if (rvm_codegen_getcodesize(co->cg)) {
			if (!compileonly) {
				fprintf(stdout, "\nExecution:\n");
				rvm_cpu_exec_debug(cpu, rvm_codegen_getcode(co->cg, 0), 0);
			}
		}
	} else {
		if (!compileonly)
			rvm_cpu_exec(cpu, rvm_codegen_getcode(co->cg, 0), 0);
	}

	if (unmapscript)
		codegen_unmap_file(unmapscript);
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

	rpa_dbex_add_callback_exact(dbex, "BitwiseANDOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseXOROp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseOROp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AdditiveExpressionOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "MultiplicativeExpressionOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ShiftExpressionOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "EqualityExpressionOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "RelationalExpressionOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalOROp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalANDOp", RPA_REASON_ALL, codegen_binary_asmop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "AssignmentOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
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
	rpa_dbex_add_callback_exact(dbex, "LogicalNotOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseNotOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "UnaryOperatorOpcode", RPA_REASON_MATCHED, codegen_opcode_unary_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PrintOp", RPA_REASON_MATCHED, codegen_printop_callback, co);


	rpa_dbex_add_callback_exact(dbex, "PostfixOperator", RPA_REASON_MATCHED, codegen_opcode_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PostfixExpressionOp", RPA_REASON_MATCHED, codegen_oppostfix_callback, co);
	rpa_dbex_add_callback_exact(dbex, "PrefixExpressionOp", RPA_REASON_MATCHED, codegen_opprefix_callback, co);


	rpa_dbex_add_callback_exact(dbex, "UnaryExpressionOp", RPA_REASON_ALL, codegen_unary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LogicalNotExpressionOp", RPA_REASON_ALL, codegen_unary_asmop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BitwiseNotExpressionOp", RPA_REASON_ALL, codegen_unary_asmop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "DecimalIntegerLiteral", RPA_REASON_MATCHED, codegen_integer_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DecimalNonIntegerLiteral", RPA_REASON_MATCHED, codegen_double_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BlockBegin", RPA_REASON_MATCHED, codegen_scopepush_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BlockEnd", RPA_REASON_MATCHED, codegen_scopepop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "DoKeyword", RPA_REASON_MATCHED, codegen_dokeyword_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IterationDo", RPA_REASON_MATCHED, codegen_iterationdo_callback, co);

	rpa_dbex_add_callback_exact(dbex, "sqstring", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "dqstring", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DoubleStringCharacters", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "SingleStringCharacters", RPA_REASON_MATCHED, codegen_string_callback, co);
	rpa_dbex_add_callback_exact(dbex, "Program", RPA_REASON_START|RPA_REASON_MATCHED, codegen_program_callback, co);
	rpa_dbex_add_callback_exact(dbex, "Initialiser", RPA_REASON_MATCHED, codegen_opinit_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmentExpressionOp", RPA_REASON_MATCHED, codegen_opassign_callback, co);

	rpa_dbex_add_callback_exact(dbex, "VariableAllocate", RPA_REASON_MATCHED, codegen_varalloc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "VariableAllocateAndInit", RPA_REASON_MATCHED, codegen_varalloc_to_ptr_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ReturnOp", RPA_REASON_MATCHED, codegen_opreturn_callback, co);

	rpa_dbex_add_callback_exact(dbex, "SwiId", RPA_REASON_MATCHED, codegen_swiid_callback, co);
	rpa_dbex_add_callback_exact(dbex, "SwiIdExist", RPA_REASON_MATCHED, codegen_swiidexist_callback, co);

	rpa_dbex_add_callback_exact(dbex, "PostfixExpressionValOp", RPA_REASON_ALL, codegen_valop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "LeftHandSideExpressionPush", RPA_REASON_MATCHED, codegen_push_r0_callback, co);

	////////////////

	rpa_dbex_add_callback_exact(dbex, "ThisOp", RPA_REASON_MATCHED, codegen_opthis_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IdentifierOp", RPA_REASON_MATCHED, codegen_opidentifier_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MemberIdentifierNameOp", RPA_REASON_MATCHED, codegen_h_arraynamelookup_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MemberExpressionBaseOp", RPA_REASON_MATCHED, codegen_push_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MemberExpressionIndexOp", RPA_REASON_MATCHED, codegen_n_arrayelementvalue_callback, co);
	rpa_dbex_add_callback_exact(dbex, "MemberExpressionNameOp", RPA_REASON_MATCHED, codegen_h_arrayelementvalue_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionNameOp", RPA_REASON_MATCHED, codegen_h_arrayelementvalue_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionBaseOp", RPA_REASON_MATCHED, codegen_push_r0_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionIndexOp", RPA_REASON_MATCHED, codegen_n_arrayelementvalue_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AddressLeftHandSideExpression", RPA_REASON_MATCHED, codegen_addressoflefthandside_callback, co);

	////////////////


	rpa_dbex_add_callback_exact(dbex, "ConditionalExpression", RPA_REASON_MATCHED, codegen_conditionalexp_callback, co);
	rpa_dbex_add_callback_exact(dbex, "NewArrayExpressionOp", RPA_REASON_MATCHED, codegen_opnewarray_callback, co);
	rpa_dbex_add_callback_exact(dbex, "compile_error", RPA_REASON_MATCHED, codegen_compile_error_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ArgumentsOp", RPA_REASON_ALL, codegen_costate_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BracketExpressionOp", RPA_REASON_ALL, codegen_costate_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ValLeftHandSideExpression", RPA_REASON_ALL, codegen_costate_callback, co);

	rpa_dbex_add_callback_exact(dbex, "FunctionNameLookupAlloc", RPA_REASON_MATCHED, codegen_funnamelookupalloc_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionName", RPA_REASON_MATCHED, codegen_fundeclname_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionDeclaration", RPA_REASON_MATCHED, codegen_fundecl_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionParameter", RPA_REASON_MATCHED, codegen_fundeclparameter_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionCallParameter", RPA_REASON_MATCHED, codegen_funcallparameter_callback, co);
	rpa_dbex_add_callback_exact(dbex, "FunctionCallName", RPA_REASON_MATCHED, codegen_funcallname_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CallExpressionOp", RPA_REASON_ALL, codegen_funcallexpression_callback, co);
	rpa_dbex_add_callback_exact(dbex, "NewKeyword", RPA_REASON_MATCHED, codegen_newkeyword_callback, co);

	rpa_dbex_add_callback_exact(dbex, "NewExpressionCallName", RPA_REASON_MATCHED, codegen_funcallname_callback, co);
	rpa_dbex_add_callback_exact(dbex, "NewExpressionCallOp", RPA_REASON_ALL, codegen_newexpressioncallop_callback, co);


	rpa_dbex_add_callback_exact(dbex, "IfConditionOp", RPA_REASON_MATCHED, codegen_ifconditionop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IfOp", RPA_REASON_MATCHED, codegen_ifop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ElseOp", RPA_REASON_MATCHED, codegen_elseop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IfElseOp", RPA_REASON_MATCHED, codegen_ifelseop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "WhileConditionOp", RPA_REASON_MATCHED, codegen_whileconditionop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IterationWhileOp", RPA_REASON_ALL, codegen_iterationwhileop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "QuestionMarkOp", RPA_REASON_MATCHED, codegen_questionmarkop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmentExpressionIfTrueOp", RPA_REASON_MATCHED, codegen_iftrueop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "AssignmentExpressionIfFalseOp", RPA_REASON_MATCHED, codegen_iffalseop_callback, co);

	rpa_dbex_add_callback_exact(dbex, "ForKeyword", RPA_REASON_MATCHED, codegen_forkeyword_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ForExpressionInitOp", RPA_REASON_END, codegen_forinitop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ForExpressionCompareOp", RPA_REASON_END, codegen_forcompareop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ForExpressionIncrementOp", RPA_REASON_END, codegen_forincrementop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "IterationForOp", RPA_REASON_MATCHED, codegen_iterationforop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "BreakOp", RPA_REASON_MATCHED, codegen_breakkeyword_callback, co);
	rpa_dbex_add_callback_exact(dbex, "ContinueOp", RPA_REASON_MATCHED, codegen_continuekeyword_callback, co);

	rpa_dbex_add_callback_exact(dbex, "SwitchExpressionOp", RPA_REASON_MATCHED, codegen_switchexpressionop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "SwitchStatementOp", RPA_REASON_ALL, codegen_switchstatementop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CaseExpressionOp", RPA_REASON_ALL, codegen_caseexpressionop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "CaseClauseOp", RPA_REASON_ALL, codegen_caseclauseop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DefaultKeywordOp", RPA_REASON_MATCHED, codegen_defaultkeywordop_callback, co);
	rpa_dbex_add_callback_exact(dbex, "DefaultClauseOp", RPA_REASON_MATCHED, codegen_defaultclauseop_callback, co);



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
