#include <stdio.h>
#include "rrefreg.h"
#include "rvmcpu.h"
#include "common.h"

static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *r = RVM_CPUREG_PTR(cpu, ins->op2);

	if (rvm_reg_gettype(r) == RVM_DTYPE_REFREG)
		r = REFREG2REGPTR(RVM_REG_GETP(r));
	if (rvm_reg_gettype(r) == RVM_DTYPE_WORD)
		fprintf(stdout, "R%d = %ld\n", ins->op2, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op2, RVM_REG_GETL(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op2, RVM_REG_GETD(r));
	else if (rvm_reg_gettype(r) == RVM_DTYPE_STRING)
		fprintf(stdout, "R%d = %s\n", ins->op2, ((rstring_t*) RVM_REG_GETP(r))->s.str);
	else
		fprintf(stdout, "R%d = Unknown type\n", ins->op2);
}


static rvm_switable_t switable[] = {
		{"print", test_swi_print_r},
		{NULL, NULL},
};

	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rvmreg_t d1 = rvm_reg_create_double(1.0);
	rvmreg_t d2 = rvm_reg_create_double(-1.0);
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create();
	
	rvmcpu_switable_add(vm, common_calltable);
	rvmcpu_switable_add(vm, switable);
	vmcode[off++] = rvm_asmp(RVM_LDRR, R0, DA, XX, &d1);
	vmcode[off++] = rvm_asm(ERVM_TYPE, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, R1, XX, rvm_cpu_getswi(vm, "print"));
	vmcode[off++] = rvm_asm(ERVM_CAST, R0, R0, DA, RVM_DTYPE_UNSIGNED);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(vm, "print"));
	VMTEST_REG(vmcode, off, 0, 3, "CAST");
	VMTEST_STATUS(vmcode, off, 0, "CAST STATUS");

	vmcode[off++] = rvm_asmp(RVM_LDRR, R0, DA, XX, &d2);
	vmcode[off++] = rvm_asm(ERVM_TYPE, R1, R0, XX, 0);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, R1, XX, rvm_cpu_getswi(vm, "print"));
	vmcode[off++] = rvm_asm(ERVM_CAST, R0, R0, DA, RVM_DTYPE_LONG);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(vm, "print"));
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, R0, XX, rvm_cpu_getswi(vm, "print"));
	VMTEST_REG(vmcode, off, 0, 1, "CAST");
	VMTEST_STATUS(vmcode, off, 0, "CAST STATUS");


	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
