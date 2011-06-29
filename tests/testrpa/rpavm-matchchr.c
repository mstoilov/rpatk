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


void code_rpa_matchchr(rpa_compiler_t *co, rpastat_t *stat)
{
	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'a'));
	VMTEST_REG(co->cg, 0, 1, "RPA_MATCHCHR_NAN 'a'");
	VMTEST_STATUS(co->cg, 0, "RPA_MATCHCHR_NAN STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_NAN, DA, XX, XX, 'b'));
	VMTEST_REG(co->cg, 0, -1, "RPA_MATCHCHR_NAN 'b'");
	VMTEST_STATUS(co->cg, RVM_STATUS_N, "RPA_MATCHCHR_NAN STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, 'a'));
	VMTEST_REG(co->cg, 0, 1, "RPA_MATCHCHR_OPT 'a'");
	VMTEST_STATUS(co->cg, 0, "RPA_MATCHCHR_OPT STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_OPT, DA, XX, XX, 'b'));
	VMTEST_REG(co->cg, 0, 0, "RPA_MATCHCHR_OPT 'b'");
	VMTEST_STATUS(co->cg, RVM_STATUS_Z, "RPA_MATCHCHR_OPT STATUS");


	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'a'));
	VMTEST_REG(co->cg, 0, 3, "RPA_MATCHCHR_MOP 'a'");
	VMTEST_STATUS(co->cg, 0, "RPA_MATCHCHR_MOP STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MOP, DA, XX, XX, 'b'));
	VMTEST_REG(co->cg, 0, 0, "RPA_MATCHCHR_MOP 'b'");
	VMTEST_STATUS(co->cg, RVM_STATUS_Z, "RPA_MATCHCHR_MOP STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MUL, DA, XX, XX, 'a'));
	VMTEST_REG(co->cg, 0, 3, "RPA_MATCHCHR_MUL 'a'");
	VMTEST_STATUS(co->cg, 0, "RPA_MATCHCHR_MUL STATUS");

	rvm_codegen_addins(co->cg, rvm_asm(RVM_MOV, R_TOP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asm(RPA_MATCHCHR_MUL, DA, XX, XX, 'b'));
	VMTEST_REG(co->cg, 0, -1, "RPA_MATCHCHR_MUL 'b'");
	VMTEST_STATUS(co->cg, RVM_STATUS_N, "RPA_MATCHCHR_MUL STATUS");

}


int main(int argc, char *argv[])
{
	rpa_compiler_t *co;
	rpastat_t *stat;
	ruinteger mainoff;
	char teststr[] = "aaaaa";

	co = rpa_compiler_create();
	stat = rpa_stat_create(NULL, 4096);
	rvm_cpu_addswitable(stat->cpu, "common_table", common_calltable);

	rpa_stat_init(stat, RPA_ENCODING_UTF8, teststr, teststr, teststr+3, NULL);

	mainoff = rvm_codegen_addins(co->cg, rvm_asml(RVM_NOP, XX, XX, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, R_TOP, DA, XX, -1));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, FP, DA, XX, 0));
	rvm_codegen_addins(co->cg, rvm_asml(RVM_MOV, SP, DA, XX, 0));


	rvm_codegen_addins(co->cg, rvm_asm(RPA_SHIFT, XX, XX, XX, 0));
	code_rpa_matchchr(co, stat);
	rvm_codegen_addins(co->cg, rvm_asm(RVM_NOP, XX, XX, XX, 0xabc));
	rvm_codegen_addins(co->cg, rvm_asm(RVM_EXT, XX, XX, XX, 0));

	rvm_cpu_exec(stat->cpu, rvm_codegen_getcode(co->cg, 0), mainoff);

	rpa_stat_destroy(stat);
	rpa_compiler_destroy(co);


	r_printf("Max alloc mem: %ld\n", r_debug_get_maxmem());
	r_printf("Leaked mem: %ld\n", r_debug_get_allocmem());
	return 0;
}
