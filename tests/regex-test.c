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
#define RPA_EMITSTART	 	RVM_OPSWI(RVM_SWI_ID(regextable, 9))
#define RPA_EMITEND			RVM_OPSWI(RVM_SWI_ID(regextable, 10))


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
} rpa_compiler_t;


typedef struct rpainput_s {
	const rchar *input;
	ruint32 wc;
	ruchar eof;
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
	r_free(co);
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
	rpainput_t * ptp = &stat->instack[tp];

	if (ptp->eof)
		return;
	ptp++;
	tp++;
	if (tp >= (rlong)stat->ip.serial) {
		rint inc = 0;
		ptp->input = stat->ip.input;
		if (ptp->input < stat->end) {
			inc = r_utf8_mbtowc(&ptp->wc, (const ruchar*)stat->ip.input, (const ruchar*)stat->end);
			stat->ip.input += inc;
			stat->ip.serial += 1;
			ptp->eof = 0;
		} else {
			ptp->wc = (ruint32)-1;
			ptp->eof = 1;
		}
	}
	RVM_CPUREG_SETL(cpu, IP, stat->instack[tp].wc);
	RVM_CPUREG_SETL(cpu, TP, tp);
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
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
		rpa_eqshift(cpu, ins);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
		if (cpu->status & RVM_STATUS_Z)
			rpa_shift(cpu, ins);
		while (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) {
			rpa_shift(cpu, ins);
		}
//		if (!(cpu->status & RVM_STATUS_Z))
//			RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, LR));
	} else if (flags == RPA_MATCH_MULTIOPT) {
		ruint matched = 0;
		while (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) {
			matched = 1;
			rpa_shift(cpu, ins);
		}
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, matched);
	} else {
		RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, TP)].wc == wc) ? 1 : 0);
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

	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, (!stat->instack[RVM_CPUREG_GETL(cpu, TP)].eof && stat->instack[tp].wc >= op1.p1 && stat->instack[tp].wc <= op1.p2) ? 1 : 0);
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


static void rpa_emitstart(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
//	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	r_printf("START: %s(%ld)\n", name.str, (rulong)tp);

}


static void rpa_emitend(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
//	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rword tplen = RVM_CPUREG_GETU(cpu, ins->op3);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	if (tplen) {
		r_printf("MATCHED: %s(%ld, %ld): %p\n", name.str, (rulong)tp, (rulong)tplen, name.str);
	} else {
		r_printf("MATCHED: %s(%ld, %ld)\n", name.str, (rulong)tp, (rulong)tplen);
	}
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
		{"RPA_EMITSTART", rpa_emitstart},
		{"RPA_EMITEND", rpa_emitend},
		{NULL, NULL},
};


void codegen_rpa_match(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_match");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(TP)|BIT(FP)|BIT(SP)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, BITS(FP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ORR, R0, R0, DA, BIT(TP)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
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
	rulong rulename;

	rvm_codegen_addstring_s(co->cg, NULL, "rpafdadf");
	rvm_codegen_addstring_s(co->cg, "5", "rpad");
	rvm_codegen_addstring_s(co->cg, NULL, "rpa_ma");
	rulename = rvm_codegen_addstring_s(co->cg, "3", "rpa_match_ab");
	rvm_codegen_addstring_s(co->cg, "4", "z");


	rvm_codegen_addlabel_s(co->cg, "rpa_match_abc");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, TP, R0, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_DEFAULT, rulename, rvm_asma(RPA_EMITSTART, DA, TP, XX, 0, -1));
	codegen_rpa_match_char(co, 'a', ' ');
	codegen_rpa_match_char(co, 'b', ' ');
	codegen_rpa_match_char(co, 'c', '*');
	codegen_rpa_match_char(co, 'd', '?');
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUB, R0, TP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ELNOT, R2, R0, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_DEFAULT, rulename, rvm_asma(RPA_EMITEND, DA, R1, R0, 0, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_codelabel_t *err;
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


	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, TP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, co->fpoff));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_NAN, DA, XX, RPA_MATCH_NONE));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_MUL, DA, XX, RPA_MATCH_MULTIPLE));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_OPT, DA, XX, RPA_MATCH_OPTIONAL));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_MOP, DA, XX, RPA_MATCH_MULTIOPT));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_abc", rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, "rpa_match", rvm_asm(RVM_BL, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	codegen_rpa_match(co);
	codegen_rpa_match_abc(co);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

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

	r_printf("Matched: %d\n", ((rpastat_t *)cpu->userdata1)->instack[RVM_CPUREG_GETL(cpu, TP)].input - ((rpastat_t *)cpu->userdata1)->input);
end:
	rpa_stat_destroy((rpastat_t *)cpu->userdata1);
	rvm_cpu_destroy(cpu);
	rpa_compiler_destroy(co);

	if (1||debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}
