#include "rvmcpu.h"
#include <stdio.h>

//#define EXECDEBUG 1


#define VMTEST_REG(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(vm, "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)

#define VMTEST_REGL(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asml(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(vm, "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)

#define VMTEST_REGD(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asmd(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asm(RVM_PRN, reg, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(vm, "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


#define VMTEST_REGP(code, index, reg, pval, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asmp(RVM_LDRR, R9, DA, XX, pval); \
		 code[index++] = rvm_asm(RVM_PRN, R9, XX, XX, 0); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(vm, "rvm_vmtest_check_reg")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


#define VMTEST_STATUS(code, index, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi_s(vm, "rvm_vmtest_check_status")), XX, XX, R0, 0); \
		 code[index++] = rvm_asm(RVM_NOP, XX, XX, XX, 0); } while (0)


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
	fprintf(stdout, "%s: %s\n", (const char*)RVM_CPUREG_GETU(vm, 10), vm->status == RVM_CPUREG_GETU(vm, 9) ? "PASSED" : "FAILED");
}


static rvm_switable_t common_calltable[] = {
	{"rvm_vmtest_check_reg", rvm_vmtest_check_reg},
	{"rvm_vmtest_check_status", rvm_vmtest_check_status},
	{NULL, NULL},
};
