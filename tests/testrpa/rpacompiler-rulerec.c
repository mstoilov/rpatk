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



void code_rpa_matchabc(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_matchabc", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'b'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'c'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


void code_rpa_matchxyz(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_matchxyz", 0);
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'x'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'y'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'z'));
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_rule_end(co);
}


void code_rpa_match_abcorxyz(rpa_compiler_t *co, rpastat_t *stat)
{
	rpa_compiler_rule_begin_s(co, "rpa_match_abcorxyz", 0);
	rpa_compiler_altexp_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_matchxyz");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_branch_begin(co, RPA_MATCH_NONE, 0);
	rpa_compiler_reference_nan_s(co, "rpa_matchabc");
	rvm_codegen_index_addrelocins(co->cg, RVM_RELOC_BRANCH, RPA_COMPILER_CURRENTEXP(co)->endidx, rvm_asm(RVM_BLES, DA, XX, XX, 0));
	rpa_compiler_branch_end(co);

	rpa_compiler_altexp_end(co);
	rpa_compiler_rule_end(co);
}


void code_rpa_matchmnode(rpa_compiler_t *co, rpastat_t *stat)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rpa_compiler_reference_mul_s(co, "rpa_match_abcorxyz");
	VMTEST_REG(co->cg, 0, 12, "RPA_MNODE_MUL 'abcorxyz'");
	VMTEST_STATUS(co->cg, 0, "RPA_MNODE_MUL STATUS");
}


int main(int argc, char *argv[])
{
	rvm_codelabel_t *err;
	rpa_compiler_t *co;
	rpastat_t *stat;
	unsigned int mainoff;
	size_t i;
	rarray_t *records = rpa_records_create();
	char teststr[] = "abcabcxyzabc";

	co = rpa_compiler_create();
	stat = rpa_stat_create(NULL, 4096);
	rvm_cpu_addswitable(stat->cpu, "common_table", common_calltable);

	rpa_stat_init(stat, RPA_ENCODING_UTF8, teststr, teststr, teststr+12, records);

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));

	code_rpa_matchmnode(co, stat);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));
	code_rpa_matchabc(co, stat);
	code_rpa_matchxyz(co, stat);
	code_rpa_match_abcorxyz(co, stat);

	if (rvm_codegen_relocate(co->cg, &err) < 0) {
		r_printf("Unresolved symbol: %s\n", err->name->str);
		goto end;
	}

	rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);

	for (i = 0; i < r_array_length(records); i++) {
		rparecord_t *rec = (rparecord_t *)r_array_slot(records, i);
		r_printf("%3d : ", i);
		if (rec->type & RPA_RECORD_START)
			r_printf("START ");
		if (rec->type & RPA_RECORD_END)
			r_printf("END ");
		r_printf("%s(%d) %d, %d\n", rec->rule, rec->type, rec->top, rec->size);
	}

end:
	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);
	rpa_records_destroy(records);

	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
