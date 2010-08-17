#include <stdio.h>
#include "common.h"


	
int main(int argc, char *argv[])
{
	ruint ret = 0;
	ruint off = 0;
	rword l_add2 = 0, l_add3 = 0, l_main = 0;
	rvm_asmins_t vmcode[256];
	rvm_cpu_t *vm = rvm_cpu_create();
	
	rvm_cpu_switable_add(vm, common_calltable);

	vmcode[off++]   = rvm_asmr(RVM_B,   DA, XX, XX, &l_main);

	/*
	 * R0 = R0 + R1
	 */
	l_add2 = off;
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R0, R1, 0);
	vmcode[off++] = rvm_asm(RVM_RET, XX, XX, XX, 0);


	/*
	 * R0 = R0 + R1 + R2
	 */
	l_add3 = off;
	vmcode[off++] = rvm_asmu(RVM_PUSHM,DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asmi(RVM_PUSH, R2, XX, XX, 0);
	vmcode[off++] = rvm_asmr(RVM_BL,   DA, XX, XX, &l_add2);
	vmcode[off++] = rvm_asmi(RVM_POP,  R1, XX, XX, 0);
	vmcode[off++] = rvm_asmr(RVM_BL,   DA, XX, XX, &l_add2);
	vmcode[off++] = rvm_asmi(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(LR));
	vmcode[off++] = rvm_asmi(RVM_RET,  XX, XX, XX, 0);
	/*
	 * We could directly restore the LR in the PC, so we will not need the RET instruction after POPM
	 *
	 * vmcode[off++] = rvm_asmi(RVM_POPM, DA, XX, XX, BIT(R7)|BIT(R8)|BIT(SP)|BIT(PC));
	 *
	 */


	l_main = off;
	vmcode[off++] = rvm_asmi(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asmi(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asmr(RVM_BL,  DA, XX, XX, &l_add2);
	VMTEST_REG(vmcode, off, 0, 3, "BL/RET");
	vmcode[off++] = rvm_asmi(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asmi(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asmi(RVM_MOV, R2, DA, XX, 4);
	vmcode[off++] = rvm_asmr(RVM_BL,  DA, XX, XX, &l_add3);
	VMTEST_REG(vmcode, off, 0, 7, "BL/RET");
	vmcode[off++] = rvm_asmi(RVM_EXT, R0, XX, XX, 0);


	rvm_relocate(vmcode, off);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	rvm_cpu_destroy(vm);
	return 0;
}
