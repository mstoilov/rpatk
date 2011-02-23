#include "rvmcpu.h"
#include <stdio.h>

#define EXECDEBUG 1


#define VMTEST_REG(code, reg, val, msg) \
	do { rvm_codegen_addins(code, rvm_asm(RVM_MOV, R8, DA, XX, reg)); \
		 rvm_codegen_addins(code, rvm_asm(RVM_MOV, R9, DA, XX, val)); \
		 rvm_codegen_addins(code, rvm_asmp(RVM_MOV, R10, DA, XX, msg)); \
		 rvm_codegen_addins(code, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(stat->cpu, "rvm_vmtest_check_reg")), XX, XX, R0, 0)); \
		 rvm_codegen_addins(code, rvm_asm(RVM_NOP, XX, XX, XX, 0)); } while (0)

#define VMTEST_STATUS(code, val, msg) \
	do { rvm_codegen_addins(code, rvm_asm(RVM_MOV, R9, DA, XX, val)); \
		 rvm_codegen_addins(code, rvm_asmp(RVM_MOV, R10, DA, XX, msg)); \
		 rvm_codegen_addins(code, rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(stat->cpu, "rvm_vmtest_check_status")), XX, XX, R0, 0)); \
		 rvm_codegen_addins(code, rvm_asm(RVM_NOP, XX, XX, XX, 0)); } while (0)


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare)
 * R8 - (0 : 7) Register to check
 */
static void rvm_vmtest_check_reg(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_CPUREG_GETU(vm, 10), RVM_CPUREG_GETU(vm, RVM_CPUREG_GETU(vm, 8)) == RVM_CPUREG_GETU(vm, 9) ? "PASSED" : "FAILED");
}


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare the status register)
 */
static void rvm_vmtest_check_status(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_CPUREG_GETU(vm, 10), (vm->status & RVM_CPUREG_GETU(vm, 9))== RVM_CPUREG_GETU(vm, 9) ? "PASSED" : "FAILED");
}


static rvm_switable_t common_calltable[] = {
	{"rvm_vmtest_check_reg", rvm_vmtest_check_reg},
	{"rvm_vmtest_check_status", rvm_vmtest_check_status},
	{NULL, NULL},
};
