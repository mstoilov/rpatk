#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rmem.h"
#include "rpacompiler.h"
#include "rpastat.h"
#include "common.h"



void code_rpa_matchabc(rpa_compiler_t *co, rpastat_t *stat)
{
	rulong ruleidx;
	const rchar *rule = "rpa_matchabc";
	const rchar *ruleend = "rpa_matchabc_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'b'));
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


void code_rpa_matchxyz(rpa_compiler_t *co, rpastat_t *stat)
{
	rulong ruleidx;
	const rchar *rule = "rpa_matchxyz";
	const rchar *ruleend = "rpa_matchxyz_end";

	ruleidx = rvm_codegen_addstring_s(co->cg, NULL, rule);
	rvm_codegen_addlabel_s(co->cg, rule);

	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_STRING, ruleidx, rvm_asm(RPA_EMITSTART, DA, R_TOP, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_PUSHM, DA, XX, XX, BIT(R_TOP)|BIT(R_WHT)|BIT(LR)));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_BRANCH, ruleend, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'y'));
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

void code_rpa_matchmnode(rpa_compiler_t *co, rpastat_t *stat)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchabc", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 3, "RPA_MNODE_NAN 'abc'");
	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_NAN STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchxyz", rvm_asm(RPA_BXLNAN, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, -1, "RPA_MNODE_NAN 'xyz'");
	VMTEST_STATUS(co->cg, RVM_STATUS_N, "RPA_MNODE_NAN STATUS");


	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchabc", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 3, "RPA_MNODE_OPT 'abc'");
	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_OPT STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchxyz", rvm_asm(RPA_BXLOPT, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 0, "RPA_MNODE_OPT 'xyz'");
	VMTEST_STATUS(co->cg, RVM_STATUS_Z, "RPA_MNODE_OPT STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchabc", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 12, "RPA_MNODE_MUL 'abc'");
	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_MUL STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchxyz", rvm_asm(RPA_BXLMUL, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, -1, "RPA_MNODE_MUL 'xyz'");
	VMTEST_STATUS(co->cg, RVM_STATUS_N, "RPA_MNODE_MUL STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchabc", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 12, "RPA_MNODE_MOP 'abc'");
	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_MOP STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_matchxyz", rvm_asm(RPA_BXLMOP, DA, XX, XX, 0));
	VMTEST_REG(co->cg, 0, 0, "RPA_MNODE_MOP 'xyz'");
	VMTEST_STATUS(co->cg, RVM_STATUS_Z, "RPA_MNODE_MOP STATUS");
}


int main(int argc, char *argv[])
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	ruint mainoff;
	char teststr[] = "abcabcabcabc";

	co = rpa_compiler_create();
	stat = rpa_stat_create(NULL, 4096);
	rvm_cpu_addswitable(stat->cpu, "common_table", common_calltable);

	rpa_stat_init(stat, teststr, teststr, teststr+12);

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RPA_SETBXLNAN, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RPA_SETBXLMUL, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RPA_SETBXLOPT, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RPA_SETBXLMOP, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	code_rpa_matchmnode(co, stat);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	code_rpa_matchabc(co, stat);
	code_rpa_matchxyz(co, stat);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);

end:
	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);


	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
