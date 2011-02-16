#include <stdio.h>
#include <stdlib.h>
#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rvmcpu.h"
#include "rmem.h"
#include "rutf.h"


static ruint regextable;
static int debuginfo = 0;
static int parseinfo = 0;
static int compileonly = 0;

#define RPA_MATCH_NONE 0
#define RPA_MATCH_MULTIPLE (1 << 0)
#define RPA_MATCH_OPTIONAL (1 << 1)
#define RPA_MATCH_MULTIOPT (RPA_MATCH_MULTIPLE | RPA_MATCH_OPTIONAL)
#define R_NAN R4
#define R_MUL R5
#define R_OPT R6
#define R_MOP R7


#define RPA_MATCHCHR 		RVM_OPSWI(RVM_SWI_ID(regextable, 0))
#define RPA_EQMATCHCHR 		RVM_OPSWI(RVM_SWI_ID(regextable, 1))
#define RPA_NEQMATCHCHR		RVM_OPSWI(RVM_SWI_ID(regextable, 2))
#define RPA_MATCHRNG 		RVM_OPSWI(RVM_SWI_ID(regextable, 3))
#define RPA_EQMATCHRNG 		RVM_OPSWI(RVM_SWI_ID(regextable, 4))
#define RPA_NEQMATCHRNG 	RVM_OPSWI(RVM_SWI_ID(regextable, 5))
#define RPA_SHIFT		 	RVM_OPSWI(RVM_SWI_ID(regextable, 6))
#define RPA_EQSHIFT		 	RVM_OPSWI(RVM_SWI_ID(regextable, 7))
#define RPA_NEQSHIFT	 	RVM_OPSWI(RVM_SWI_ID(regextable, 8))


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
} rpa_compiler_t;


typedef struct rpainput_s {
	ruint32 wc;
	const rchar *input;
} rpainput_t;


typedef struct rpainmap_s {
	const rchar *input;
	rulong serial;
} rpainmap_t;


typedef struct rpastat_s {
	const rchar *input;
	const rchar *start;
	const rchar *end;
	ruint error;
	rpainput_t *instack;
	rulong instacksize;
	rulong cursize;
	rpainmap_t ip;
} rpastat_t;


rpa_compiler_t *rpa_compiler_create()
{
	rpa_compiler_t *co;

	co = r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->cg = rvm_codegen_create();
	co->scope = rvm_scope_create();
	return co;
}


void rpa_compiler_destroy(rpa_compiler_t *co)
{
	if (co) {
		rvm_codegen_destroy(co->cg);
		rvm_scope_destroy(co->scope);
	}
}

rpastat_t *rpa_stat_create()
{
	rpastat_t *stat = (rpastat_t *) r_zmalloc(sizeof(*stat));
	return stat;
}


int rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
{
	rulong size;

	if (start > end) {

		return -1;
	}
	if (input < start || input > end) {

		return -1;
	}
	size = end - start;
	stat->start = start;
	stat->end = end;
	stat->input = input;
	stat->error = 0;
	stat->cursize = 0;
	if (stat->instacksize < size) {
		stat->instack = r_realloc(stat->instack, (size + 1) * sizeof(rpainput_t));
		stat->instacksize = size + 1;
	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	return 0;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat->instack)
		r_free(stat->instack);
	r_free(stat);
}


static void rpa_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong tp = RVM_CPUREG_GETL(cpu, TP);

	if (stat->ip.input >= stat->end) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, 1);
		return;
	}

	tp += 1;
	if (tp >= (rlong)stat->ip.serial) {
		rint inc;
		inc = r_utf8_mbtowc(&stat->instack[tp].wc, (const ruchar*)stat->ip.input, (const ruchar*)stat->end);
		stat->instack[tp].input = stat->ip.input;
		stat->ip.input += inc;
		stat->ip.serial += inc;
	}
	RVM_CPUREG_SETL(cpu, IP, stat->instack[tp].wc);
	RVM_CPUREG_SETL(cpu, TP, tp);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, 0);
}


static void rpa_eqshift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (cpu->status & RVM_STATUS_Z)
		rpa_shift(cpu, ins);
}


static void rpa_neqshift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!(cpu->status & RVM_STATUS_Z))
		rpa_shift(cpu, ins);
}


static void rpa_matchchr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	rword flags = RVM_CPUREG_GETU(cpu, ins->op2);

	if (flags == RPA_MATCH_OPTIONAL) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!(cpu->status & RVM_STATUS_V) && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
		rpa_eqshift(cpu, ins);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!(cpu->status & RVM_STATUS_V) && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
		if (cpu->status & RVM_STATUS_Z)
			rpa_shift(cpu, ins);
		while (!(cpu->status & RVM_STATUS_V) && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) {
			rpa_shift(cpu, ins);
		}
//		if (!(cpu->status & RVM_STATUS_Z))
//			RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, LR));
	} else if (flags == RPA_MATCH_MULTIOPT) {
		ruint matched = 0;
		while (!(cpu->status & RVM_STATUS_V) && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) {
			matched = 1;
			rpa_shift(cpu, ins);
		}
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, matched);
	} else {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!(cpu->status & RVM_STATUS_V) && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
		rpa_eqshift(cpu, ins);
//		if (!(cpu->status & RVM_STATUS_Z))
//			RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, LR));
	}
}


static void rpa_eqmatchchr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (cpu->status & RVM_STATUS_Z)
		rpa_matchchr(cpu, ins);
}


static void rpa_neqmatchchr(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!(cpu->status & RVM_STATUS_Z))
		rpa_matchchr(cpu, ins);
}


static void rpa_matchrng(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong tp = RVM_CPUREG_GETL(cpu, TP);
	rpair_t op1 = RVM_CPUREG_GETPAIR(cpu, ins->op1);

	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!(cpu->status & RVM_STATUS_V) && stat->instack[tp].wc >= op1.p1 && stat->instack[tp].wc <= op1.p2) ? 1 : 0);
}


static void rpa_eqmatchrng(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (cpu->status & RVM_STATUS_Z)
		rpa_matchrng(cpu, ins);
}


static void rpa_neqmatchrng(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (!(cpu->status & RVM_STATUS_Z))
		rpa_matchrng(cpu, ins);
}




static rvm_switable_t switable[] = {
		{"RPA_MATCHCHR", rpa_matchchr},
		{"RPA_EQMATCHCHR", rpa_eqmatchchr},
		{"RPA_NEQMATCHCHR", rpa_neqmatchchr},
		{"RPA_MATCHCHR", rpa_matchrng},
		{"RPA_EQMATCHCHR", rpa_eqmatchrng},
		{"RPA_NEQMATCHCHR", rpa_neqmatchrng},
		{"RPA_SHIFT", rpa_shift},
		{"RPA_EQSHIFT", rpa_eqshift},
		{"RPA_NEQSHIFT", rpa_neqshift},
		{NULL, NULL},
};


void codegen_rpa_match(rpa_compiler_t *co)
{
	rulong off, l1, l2;

	rvm_scope_addoffset_s(co->scope, "rpa_match", co->fpoff);
	l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R1, FP, DA, co->fpoff++));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, PC, DA, sizeof(rvm_asmins_t) * 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	l2 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0)); 							/* Will be re-written later */
	rvm_codemap_addoffset_s(co->cg->codemap, rvm_codemap_lookup_s(co->cg->codemap, ".code"), "rpa_match", rvm_codegen_getcodesize(co->cg));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(TP)|BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, BITS(FP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ORR, R0, R0, DA, BIT(TP)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	off = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_replaceins(co->cg, l2, rvm_asm(RVM_B, DA, XX, XX, off - l2));
}


void codegen_rpa_match_char(rpa_compiler_t *co, rword wc, rchar q)
{
	if (q == '?') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR, DA, R_OPT, XX, wc));
	} else if (q == '+') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR, DA, R_MUL, XX, wc));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	} else if (q == '*') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR, DA, R_MOP, XX, wc));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR, DA, R_NAN, XX, wc));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	}
}


void codegen_rpa_match_abc(rpa_compiler_t *co)
{
	rulong off, l1, l2;

	rvm_scope_addoffset_s(co->scope, "rpa_match_abc", co->fpoff);
	l1 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDRS, R1, FP, DA, co->fpoff++));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, PC, DA, sizeof(rvm_asmins_t) * 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SETTYPE, R0, DA, XX, RVM_DTYPE_FUNCTION));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_STRR, R0, R1, XX, 0));
	l2 = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_B, DA, XX, XX, 0)); 							/* Will be re-written later */
	rvm_codemap_addoffset_s(co->cg->codemap, rvm_codemap_lookup_s(co->cg->codemap, ".code"), "rpa_match_abc", rvm_codegen_getcodesize(co->cg));
	codegen_rpa_match_char(co, 'a', ' ');
	codegen_rpa_match_char(co, 'b', '?');
	codegen_rpa_match_char(co, 'c', '+');
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	off = rvm_codegen_getcodesize(co->cg);
	rvm_codegen_replaceins(co->cg, l2, rvm_asm(RVM_B, DA, XX, XX, off - l2));


}


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_codelabel_t *unresolved;
	rpa_compiler_t *co;
	rint i;

	co = rpa_compiler_create();
	cpu = rvm_cpu_create_default();
	cpu->userdata1 = rpa_stat_create();
	regextable = rvm_cpu_addswitable(cpu, switable);

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-L") == 0) {
		} else if (r_strcmp(argv[i], "-d") == 0) {
			debuginfo = 1;
		} else if (r_strcmp(argv[i], "-c") == 0) {
			compileonly = 1;
		} else if (r_strcmp(argv[i], "-p") == 0) {
			parseinfo = 1;
		}
	}

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-e") == 0) {
			if (++i < argc) {
				rstr_t bnfexpr = { argv[i], r_strlen(argv[i]) };
				rpa_stat_init((rpastat_t *)cpu->userdata1, bnfexpr.str, bnfexpr.str, bnfexpr.str + bnfexpr.size);
			}
		}
	}


	codegen_rpa_match(co);
	codegen_rpa_match_abc(co);

	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, TP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, co->fpoff));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_NAN, DA, XX, RPA_MATCH_NONE));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_MUL, DA, XX, RPA_MATCH_MULTIPLE));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_OPT, DA, XX, RPA_MATCH_OPTIONAL));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_MOP, DA, XX, RPA_MATCH_MULTIOPT));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R4, DA, XX, rvm_scope_lookup_s(co->scope, "rpa_match")->data.offset));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_LDS, R0, DA, XX, rvm_scope_lookup_s(co->scope, "rpa_match_abc")->data.offset));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R4, XX, XX, 0));
//	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, R0, PC, DA, (rvm_codegen_getcodesize(co->cg) - rvm_codemap_lookup_s(co->cg->codemap, "rpa_match_abc")->loc.index  + 1) * sizeof(rvm_asmins_t)));
//	rvm_codegen_addins(co->cg, rvm_asmx(RVM_BL, DA, XX, XX, rvm_codemap_lookup_s(co->cg->codemap, "rpa_match")));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

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

end:
	rpa_stat_destroy((rpastat_t *)cpu->userdata1);
	rvm_cpu_destroy(cpu);

	if (debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}
