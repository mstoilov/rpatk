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


void code_rpa_match_num(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_num");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_var(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_var");

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'a', 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, 'A', 'Z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_mulop(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_mulop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_mulexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '*'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_num", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_var", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}


void code_rpa_match_divop(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_divop");

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_mulexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '/'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_altexp_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_num", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_var", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_altexp_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_rule_end(co);
}



void code_rpa_match_mulexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_mulexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_mulop", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_divop", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);


	rpa_compiler_nonloopybranch_begin(co);
	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm2(RPA_MATCHRNG_NAN, DA, XX, XX, '0', '9'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_MULTIPLE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rpa_compiler_loop_end(co);
}


void code_rpa_match_addexp(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_loop_begin_s(co, "rpa_match_addexp");
	rpa_compiler_altexp_begin(co);

	rpa_compiler_branch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_addexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_class_begin(co);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '+'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, '-'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BGRE, DA, XX, XX, 0));
	rpa_compiler_class_end(co, RPA_MATCH_NONE);
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_mulexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co, RPA_MATCH_NONE);


	rpa_compiler_nonloopybranch_begin(co);
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_mulexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_nonloopybranch_end(co, RPA_MATCH_NONE);

	rpa_compiler_altexp_end(co, RPA_MATCH_NONE);
	rpa_compiler_loop_end(co);
}


void code_rpa_matchmnode(rpa_compiler_t *co, rpastat_t *stat)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));

	rpa_compiler_exp_begin(co);

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpa_match_addexp", rvm_asm(RPA_BXLWHT, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));

	rpa_compiler_exp_end(co, RPA_MATCH_NONE);


//	VMTEST_REG(co->cg, 0, 12, "RPA_MNODE_NAN 'aloop'");
//	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_NAN STATUS");
}


void rpa_record_dump(FILE *file, rint serial, rparecord_t *rec, rpastat_t *stat)
{
	rchar buf[240];
	rint bufsize = sizeof(buf) - 1;
	rint n = 0, size;

	r_memset(buf, 0, bufsize);

	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%3d ( %7ld ): ", serial, rec->ruleid);
	if (rec->type & RPA_RECORD_START)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "START ");
	if (rec->type & RPA_RECORD_MATCH)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "MATCH ");
	if (rec->type & RPA_RECORD_END)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "END ");
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%s(%d) ", rec->rule, rec->type);

	r_memset(buf + n, ' ', bufsize - n);
	n = 55;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : %5d, %3d", rec->top, rec->size);


	r_memset(buf + n, ' ', bufsize - n);
	n = 75;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : ");
	size = stat->instack[rec->top + rec->size].input - stat->instack[rec->top].input;
	if (size >= bufsize - n - 1)
		size = bufsize - n - 1;
	if (rec->type & RPA_RECORD_END) {
		r_strncpy(buf + n, stat->instack[rec->top].input, size);
		n += size;
		buf[n] = '\0';
	}

	fprintf(file, "%s\n", buf);
}


int main(int argc, char *argv[])
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	ruint mainoff;
	rint i;
	char teststr[] = "1*v23*z457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33*23*457/89+233*2/33 ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
//	char teststr[] = "1*23ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";

	co = rpa_compiler_create();
	stat = rpa_stat_create(4096);
	rvm_cpu_addswitable(stat->cpu, common_calltable);

	rpa_stat_init(stat, teststr, teststr, teststr+90);

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));

	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));

	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_nan", rvm_asm(RVM_MOV, R_MNODE_NAN, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mul", rvm_asm(RVM_MOV, R_MNODE_MUL, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_opt", rvm_asm(RVM_MOV, R_MNODE_OPT, DA, XX, 0));
	rvm_codegen_addrelocins_s(co->cg, RVM_RELOC_JUMP, "rpacompiler_mnode_mop", rvm_asm(RVM_MOV, R_MNODE_MOP, DA, XX, 0));

	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	code_rpa_matchmnode(co, stat);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	code_rpa_match_mulexp(co, stat);
	code_rpa_match_addexp(co, stat);
	code_rpa_match_divop(co, stat);
	code_rpa_match_mulop(co, stat);
	code_rpa_match_num(co, stat);
	code_rpa_match_var(co, stat);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	for (i = 0; i < 1000; i++) {
		stat->cache.reclen = 0;
//		rpa_stat_init(stat, teststr, teststr, teststr+90);
//		r_array_setlength(stat->records, 0);
		rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);
	}
	for (i = 0; 0 && i < r_array_length(stat->records); i++) {
		rparecord_t *rec = (rparecord_t *)r_array_slot(stat->records, i);
		rpa_record_dump(stdout, i, rec, stat);
	}

end:
	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);


	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
