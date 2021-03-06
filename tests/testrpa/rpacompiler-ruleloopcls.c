#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "rlib/rmem.h"
#include "rpa/rpacompiler.h"
#include "rpa/rpastatpriv.h"
#include "common.h"

/*
 * This test is broken now, this is indirect loop which has to be fixed first
 * before attempting this test. Also some of the rules are incorrectly defined,
 * for example 'mulop' should be defined as a loopy rule.
 */
void code_rpa_match_num(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_num", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_var(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_var", 0);

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);

	rpa_compiler_class_begin(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_term(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_term", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_num");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_var");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '('));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_xorexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, ')'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_branch_end(co);


	rpa_compiler_altexp_end(co);

	rpa_compiler_rule_end(co);
}


void code_rpa_match_mulop(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_mulop", 0);

	rpa_compiler_reference_nan_s(co, "rpa_match_mulexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '*'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_term");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_divop(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_divop", 0);

	rpa_compiler_reference_nan_s(co, "rpa_match_mulexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co, RPA_MATCH_NONE);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '/'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_term");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}



void code_rpa_match_mulexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_mulexp");
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_mulop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_divop");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_nonloopybranch_begin(co, RPA_MATCH_NONE);
	rpa_compiler_reference_nan_s(co, "rpa_match_term");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_loop_end(co);
}


void code_rpa_match_addexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_addexp");
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_addexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_mulexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_addexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '+'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_mulexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);


	rpa_compiler_nonloopybranch_begin(co, RPA_MATCH_NONE);
	rpa_compiler_reference_nan_s(co, "rpa_match_mulexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_loop_end(co);
}


void code_rpa_match_orexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_orexp");
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '|'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_addexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_nonloopybranch_begin(co, RPA_MATCH_NONE);
	rpa_compiler_reference_nan_s(co, "rpa_match_addexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_loop_end(co);
}


void code_rpa_match_andexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_andexp");
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_andexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '&'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_nonloopybranch_begin(co, RPA_MATCH_NONE);
	rpa_compiler_reference_nan_s(co, "rpa_match_orexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_loop_end(co);
}


void code_rpa_match_xorexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_xorexp");
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_match_xorexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '^'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_reference_nan_s(co, "rpa_match_andexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_nonloopybranch_begin(co, RPA_MATCH_NONE);
	rpa_compiler_reference_nan_s(co, "rpa_match_andexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_loop_end(co);
}


void code_rpa_matchmnode(rpa_compiler_t *co, rpastat_t *stat)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));

	rpa_compiler_exp_begin(co, RPA_MATCH_NONE, 0);

	rpa_compiler_reference_nan_s(co, "rpa_match_xorexp");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_end(co);


//	VMTEST_REG(co->cg, 0, 12, "RPA_MNODE_NAN 'aloop'");
//	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_NAN STATUS");
}


int main(int argc, char *argv[])
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	unsigned int mainoff;
	int i;
	char teststr[] = "1*v23*z457/(((((((((((((89^233)))))))))*2)/33)+23)*457)/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33|5&77^37 ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
//	char teststr[] = "1/23+45##ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";

	co = rpa_compiler_create();
	stat = rpa_stat_create(NULL, 4096);
	rvm_cpu_addswitable(stat->cpu, "common_table", common_calltable);

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));

	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	code_rpa_matchmnode(co, stat);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	code_rpa_match_xorexp(co, stat);
	code_rpa_match_andexp(co, stat);
	code_rpa_match_orexp(co, stat);
	code_rpa_match_addexp(co, stat);
	code_rpa_match_mulexp(co, stat);
	code_rpa_match_divop(co, stat);
	code_rpa_match_mulop(co, stat);
	code_rpa_match_num(co, stat);
	code_rpa_match_var(co, stat);
	code_rpa_match_term(co, stat);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	for (i = 0; i < 1000; i++) {
		rpa_stat_init(stat, RPA_ENCODING_UTF8, teststr, teststr, teststr+170);
		rpa_stat_cachedisable(stat, 0);

		rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);
	}
	for (i = 0; i < r_array_length(stat->records); i++) {
		rpa_record_dump(stat->records, i, 0);
	}

	r_printf("(%s) Matched size: %s\n", argv[0], RVM_CPUREG_GETU(stat->cpu, R0) == 151 ? "PASSED" : "FAILED");
	r_printf("(%s) Records size: %s(cache hits: %d)\n", argv[0], r_array_length(stat->records) == 500 ? "PASSED" : "FAILED", stat->cache->hit);

end:
	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);


	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
