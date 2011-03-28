#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rvmcodegen.h"
#include "rvmscope.h"
#include "rvmcpu.h"
#include "rmem.h"
#include "rutf.h"


static ruint regextable;
static int debuginfo = 0;
static int parseinfo = 0;
static int compileonly = 0;

#define RPA_RECORD_NONE (0)
#define RPA_RECORD_START (1 << 0)
#define RPA_RECORD_END (1 << 1)
#define RPA_RECORD_MATCH (1 << 2)

#define RPA_MATCH_NONE 0
#define RPA_MATCH_MULTIPLE (1 << 0)
#define RPA_MATCH_OPTIONAL (1 << 1)
#define RPA_MATCH_MULTIOPT (RPA_MATCH_MULTIPLE | RPA_MATCH_OPTIONAL)
#define R_FLG R7
#define R_ARG R8
#define R_WHT FP
#define R_TOP TP



#define R_MNODE_NAN R4
#define R_MNODE_MUL R5
#define R_MNODE_OPT R6
#define R_MNODE_MOP R7


#define RPA_MATCHCHR_NAN	RVM_OPSWI(RVM_SWI_ID(regextable, 0))
#define RPA_MATCHCHR_OPT	RVM_OPSWI(RVM_SWI_ID(regextable, 1))
#define RPA_MATCHCHR_MUL	RVM_OPSWI(RVM_SWI_ID(regextable, 2))
#define RPA_MATCHCHR_MOP	RVM_OPSWI(RVM_SWI_ID(regextable, 3))
#define RPA_SHIFT		 	RVM_OPSWI(RVM_SWI_ID(regextable, 4))
#define RPA_EQSHIFT		 	RVM_OPSWI(RVM_SWI_ID(regextable, 5))
#define RPA_NEQSHIFT	 	RVM_OPSWI(RVM_SWI_ID(regextable, 6))
#define RPA_EMITSTART	 	RVM_OPSWI(RVM_SWI_ID(regextable, 7))
#define RPA_EMITEND			RVM_OPSWI(RVM_SWI_ID(regextable, 8))
#define RPA_MATCHANY_NAN	RVM_OPSWI(RVM_SWI_ID(regextable, 9))
#define RPA_MATCHEOL_NAN	RVM_OPSWI(RVM_SWI_ID(regextable, 10))
#define RPA_BXLWHT			RVM_OPSWI(RVM_SWI_ID(regextable, 11))


typedef struct rpa_compiler_s {
	rvm_codegen_t *cg;
	rboolean optimized;
	rvm_scope_t *scope;
	rulong fpoff;
} rpa_compiler_t;


typedef struct rparecord_s {
	rlist_t head;
	rlink_t lnk;
	const char *rule;
	rword top;
	rword size;
	rword type;
} rparecord_t;


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
	rarray_t *records;
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
	stat->records = r_array_create(sizeof(rparecord_t));
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
	r_array_setlength(stat->records, 0);
	return 0;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat->instack)
		r_free(stat->instack);
	r_object_destroy((robject_t*)stat->records);
	r_free(stat);
}


static void rpa_shift(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rlong tp = RVM_CPUREG_GETL(cpu, R_TOP);
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
	RVM_CPUREG_SETL(cpu, R_TOP, tp);
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


static void rpa_matchchr_do(rvmcpu_t *cpu, rvm_asmins_t *ins, rword flags)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rword wc = RVM_CPUREG_GETU(cpu, ins->op1);
	rword matched = 0;

	if (flags == RPA_MATCH_OPTIONAL) {
		if (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc == wc) {
			rpa_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched);
	} else if (flags == RPA_MATCH_MULTIPLE) {
		while (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc == wc) {
			rpa_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	} else if (flags == RPA_MATCH_MULTIOPT) {
		while (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc == wc) {
			rpa_shift(cpu, ins);
			matched += 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_Z;
		RVM_CPUREG_SETU(cpu, R0, matched );
	} else {
		if (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc == wc) {
			rpa_shift(cpu, ins);
			matched = 1;
		}
		cpu->status = matched ? 0 : RVM_STATUS_N;
		RVM_CPUREG_SETU(cpu, R0, matched ? matched : (rword)-1);
	}

}


static void rpa_matchany_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;

	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof) ? 0 : 1);
	if (!(cpu->status & RVM_STATUS_N))
		rpa_shift(cpu, ins);
}


static void rpa_matcheol_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;

	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && r_strchr("\r\n", stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc)) ? 0 : 1);
	if (!(cpu->status & RVM_STATUS_N))
		rpa_shift(cpu, ins);
}


static void rpa_matchchr_nan(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
//	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
//	rword wc = RVM_CPUREG_GETU(cpu, ins->op1);
//
//	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, (!stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].eof && stat->instack[RVM_CPUREG_GETL(cpu, R_TOP)].wc == wc) ? 0 : 1);
//	if (!(cpu->status & RVM_STATUS_N))
//		rpa_shift(cpu, ins);

	rpa_matchchr_do(cpu, ins, RPA_MATCH_NONE);
}


static void rpa_matchchr_opt(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_matchchr_do(cpu, ins, RPA_MATCH_OPTIONAL);
}


static void rpa_matchchr_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_matchchr_do(cpu, ins, RPA_MATCH_MULTIPLE);
}


static void rpa_matchchr_mop(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpa_matchchr_do(cpu, ins, RPA_MATCH_MULTIOPT);
}


static void rpa_emitstart(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rlong index;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	index = r_array_add(stat->records, NULL);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->type = RPA_RECORD_START;
//	r_printf("START: %s(%ld)\n", name.str, (rulong)tp);
}


static void rpa_emitend(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rpastat_t *stat = (rpastat_t *)cpu->userdata1;
	rparecord_t *rec;
	rlong index;
	rword tp = RVM_CPUREG_GETU(cpu, ins->op2);
	rword tplen = RVM_CPUREG_GETU(cpu, ins->op3);
	rstr_t name = {RVM_CPUREG_GETSTR(cpu, ins->op1), RVM_CPUREG_GETSIZE(cpu, ins->op1)};

	index = r_array_add(stat->records, NULL);
	rec = (rparecord_t *)r_array_slot(stat->records, index);
	rec->rule = name.str;
	rec->top = tp;
	rec->size = tplen;
	rec->type = RPA_RECORD_START;

	if (tplen) {
		rec->type = RPA_RECORD_END | RPA_RECORD_MATCH;
//		r_printf("MATCHED: %s(%ld, %ld): %p(%d)\n", name.str, (rulong)tp, (rulong)tplen, name.str, name.size);
	} else {
		rec->type = RPA_RECORD_END;
//		r_printf("MATCHED: %s(%ld, %ld)\n", name.str, (rulong)tp, (rulong)tplen);
	}
}


static void rpa_bxlwht(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rword wht = RVM_CPUREG_GETU(cpu, ins->op2);

	RVM_CPUREG_SETU(cpu, R_WHT, wht);
	RVM_CPUREG_SETIP(cpu, LR, RVM_CPUREG_GETIP(cpu, PC));
	RVM_CPUREG_SETIP(cpu, PC, RVM_CPUREG_GETIP(cpu, ins->op1));
}


static rvm_switable_t switable[] = {
		{"RPA_MATCHCHR_NAN", rpa_matchchr_nan},
		{"RPA_MATCHCHR_OPT", rpa_matchchr_opt},
		{"RPA_MATCHCHR_MUL", rpa_matchchr_mul},
		{"RPA_MATCHCHR_MOP", rpa_matchchr_mop},
		{"RPA_SHIFT", rpa_shift},
		{"RPA_EQSHIFT", rpa_eqshift},
		{"RPA_NEQSHIFT", rpa_neqshift},
		{"RPA_EMITSTART", rpa_emitstart},
		{"RPA_EMITEND", rpa_emitend},
		{"RPA_MATCHANY_NAN", rpa_matchany_nan},
		{"RPA_MATCHEOL_NAN", rpa_matcheol_nan},
		{"RPA_BXLWHT", rpa_bxlwht},
		{NULL, NULL},
};


void codegen_rpa_match(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_match");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R_TOP, R_TOP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_mul(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_match_mul");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, DA, XX, 0));
	rvm_codegen_addlabel_s(co->cg, "rpa_match_mul_again");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R1, XX, XX, 0)); 		// Ret
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0)); 		// Ret
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 4));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R_TOP, R_TOP, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R1, R1, R0, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, "rpa_match_mul_again", rvm_asm(RVM_B, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, R1, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_char(rpa_compiler_t *co, rword wc, rchar q)
{
	if (q == '?') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, wc));
	} else if (q == '+') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MUL, DA, XX, XX, wc));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	} else if (q == '*') {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, wc));
	} else {
		rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, wc));
		rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	}
}


void codegen_rpa_match_mnode(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_match_mnode");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BLES, DA, XX, XX, 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TST, R0, R_FLG, DA, RPA_MATCH_MULTIPLE));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, -4));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SWP, R1, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TST, R_FLG, DA, XX, RPA_MATCH_OPTIONAL));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDS, R0, R0, DA, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_mnode_nan(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_mnode_nan");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, R_WHT, XX, XX, 0));
}


void codegen_rpa_mnode_opt(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_mnode_opt");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXGRE, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CMP, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_mnode_mul(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_mnode_mul");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -5));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDS, R0, R1, DA, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
}


void codegen_rpa_mnode_mop(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_mnode_mop";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);

	rvm_codegen_addlabel_s(co->cg, "rpa_mnode_mop");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, 4));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, R0, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_CLR, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADD, R0, R0, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSH, R0, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXL, R_WHT, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, R1, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_CMP, R0, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BGRE, DA, XX, XX, -5));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_ADDS, R0, R1, DA, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POP, PC, XX, XX, 0));
}


void codegen_rpa_match_aorb(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_aorb";
	const rchar *ruleend = "rpa_match_aorb_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'b'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_xyz(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_xyz";
	const rchar *ruleend = "rpa_match_xyz_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_squared", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'y'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'z'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_abc(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_abc";
	const rchar *ruleend = "rpa_match_abc_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'b'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'c'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));


	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void codegen_rpa_match_xyzorabc(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_xyzorabc";
	const rchar *ruleend = "rpa_match_xyzorabc_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));


	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_xyz", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_abc", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BGRE, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}



void codegen_rpa_match_squared(rpa_compiler_t *co)
{
	rulong ruleidx;
	const rchar *rule = "rpa_match_squared";
	const rchar *ruleend = "rpa_match_squared_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '['));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_xyzorabc", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_aorb", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ']'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matcheol_char", rvm_asm(RVM_MOV, R_WHT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, "rpa_mnode_mop", rvm_asm(RVM_BL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R1)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITEND, DA, R1, R0, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addlabel_s(co->cg, ruleend);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}




/*
 * R0 Char
 * R1 Flags
 * Return:
 * R0 = -1 No match
 * R0 = 0 Didn't match, but it was optional
 * R0 > 0 matched R0 TPs
 */

void rpa_match_char(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_match_char");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, FP, SP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, R_ARG, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, 3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TST, R0, R_FLG, DA, RPA_MATCH_MULTIPLE));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BNEQ, DA, XX, XX, -3));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R1, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, SP, FP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_POPM, DA, XX, XX, BITS(R_TOP,LR)));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SWP, R1, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_SUBS, R0, R_TOP, R1, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BEQ, DA, XX, XX, 2));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_TST, R0, R_FLG, DA, RPA_MATCH_OPTIONAL));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXNEQ, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void rpa_matchonly_char(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_matchonly_char");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, R_ARG, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void rpa_matchany_char(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_matchany_char");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHANY_NAN, R_ARG, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
}


void rpa_matcheol_char(rpa_compiler_t *co)
{
	rvm_codegen_addlabel_s(co->cg, "rpa_matcheol_char");
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHEOL_NAN, R_ARG, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BXLES, LR, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R0, DA, XX, 1));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_BX, LR, XX, XX, 0));
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
	rstr_t *script = NULL, *unmapscript = NULL;
	rvmcpu_t *cpu;
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	rint i;

	co = rpa_compiler_create();
	cpu = rvm_cpu_create_default();
	cpu->userdata1 = stat = rpa_stat_create();
	regextable = rvm_cpu_addswitable(vm, "switable", switable);

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

	for (i = 1; i < argc; i++) {
		if (r_strcmp(argv[i], "-f") == 0) {
			if (++i < argc) {
				script = codegen_map_file(argv[i]);
				if (script) {
					rpa_stat_init((rpastat_t *)cpu->userdata1, script->str, script->str, script->str + script->size);
					unmapscript = script;
				}
			}
			goto exec;
		}
	}


exec:

	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, co->fpoff));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));


//	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_FLG, DA, XX, RPA_MATCH_NONE));
//	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_squared", rvm_asm(RVM_MOV, R_WHT, DA, XX, 0));
//	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, "rpa_mnode_mul", rvm_asm(RVM_BL, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_squared", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));


//	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_xyz_p", rvm_asm(RVM_MOV, R_WHT, DA, XX, 0));
//	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, "rpa_match_mnode", rvm_asm(RVM_BL, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rpa_match_char(co);
	rpa_matchonly_char(co);
	codegen_rpa_match_abc(co);
	codegen_rpa_match_xyz(co);
	codegen_rpa_match_xyzorabc(co);
	codegen_rpa_match_aorb(co);
	codegen_rpa_match_squared(co);
	codegen_rpa_match_mnode(co);
	codegen_rpa_match(co);
	codegen_rpa_match_mul(co);
	rpa_matcheol_char(co);
	rpa_matchany_char(co);

	codegen_rpa_mnode_nan(co);
	codegen_rpa_mnode_opt(co);
	codegen_rpa_mnode_mul(co);
	codegen_rpa_mnode_mop(co);


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

	r_printf("Matched: %d\n", RVM_CPUREG_GETU(cpu, R0));
end:

	for (i = 0; 0 && i < r_array_length(stat->records); i++) {
		rparecord_t *rec = (rparecord_t *)r_array_slot(stat->records, i);
		if (rec->type & RPA_RECORD_MATCH) {
			r_printf("%d: rule: %s(%d, %d)\n", i, rec->rule, (rint)rec->top, (rint)rec->size);
		}
	}

	rpa_stat_destroy((rpastat_t *)cpu->userdata1);
	rvm_cpu_destroy(cpu);
	rpa_compiler_destroy(co);
	if (unmapscript)
		codegen_unmap_file(unmapscript);


	if (1||debuginfo) {
		r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
		r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	}
	return 0;
}
