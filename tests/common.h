#include "rvm.h"
#include <stdio.h>

//#define EXECDEBUG 1
#define VMTEST_CHECK_REG 0
#define VMTEST_CHECK_STATUS 1

#define VMTEST_REG(code, index, reg, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R8, DA, XX, reg); \
		 code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_SWI, DA, XX, R0, rvm_cpu_getswi(vm, "rvm_vmtest_check_reg")); } while (0)

#define VMTEST_STATUS(code, index, val, msg) \
	do { code[index++] = rvm_asm(RVM_MOV, R9, DA, XX, val); \
		 code[index++] = rvm_asmp(RVM_MOV, R10, DA, XX, msg); \
		 code[index++] = rvm_asm(RVM_SWI, DA, XX, R0, rvm_cpu_getswi(vm, "rvm_vmtest_check_status")); } while (0)


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare)
 * R8 - (0 : 7) Register to check
 */
static void rvm_vmtest_check_reg(rvm_cpu_t *vm)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_GET_REGU(vm, 10), RVM_GET_REGU(vm, RVM_GET_REGU(vm, 8)) == RVM_GET_REGU(vm, 9) ? "PASSED" : "FAILED");
}


/*
 * R10 - (const char*) msg
 * R9 - argument (to compare the status register)
 */
static void rvm_vmtest_check_status(rvm_cpu_t *vm)
{
	fprintf(stdout, "%s: %s\n", (const char*)RVM_GET_REGU(vm, 10), vm->status == RVM_GET_REGU(vm, 9) ? "PASSED" : "FAILED");
}


static rvm_switable_t common_calltable[] = {
	{"rvm_vmtest_check_reg", rvm_vmtest_check_reg},
	{"rvm_vmtest_check_status", rvm_vmtest_check_status},
	{NULL, NULL},
};
