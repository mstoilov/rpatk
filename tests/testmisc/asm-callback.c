#include <stdio.h>
#include "common.h"


static void rvm_callback_one(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}


static void rvm_callback_two(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}


static rvm_switable_t calltable[] = {
	{"rvm_callback_one", rvm_callback_one},
	{"rvm_callback_two", rvm_callback_two},
	{NULL, NULL},
};
	
	
	
int main(int argc, char *argv[])
{
	rint table1, table2, table3;
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();

	table1 = rvm_cpu_addswitable(vm, "calltable", calltable);
	table2 = rvm_cpu_addswitable(vm, "calltable", calltable);
	table3 = rvm_cpu_addswitable(vm, "calltable", calltable);
	if (table2 != 0 || table3 != 0) {
		fprintf(stdout, "rvm_cpu_addswitable: FAILED\n");
	}
	rvm_cpu_addswitable(vm, "common_table", common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "calltable", "rvm_callback_one")), XX, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup_s(vm, "calltable", "rvm_callback_two")), XX, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_OPSWI(RVM_SWI_ID(table1, 0)), XX, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
	fprintf(stdout, "sizeof rvm_asmins_t is: %d:\n", (unsigned int) sizeof(rvm_asmins_t));
	fprintf(stdout, "Code List (sizeof rvmreg_t is: %d(%d)):\n", (unsigned int) sizeof(rvmreg_t), (unsigned int) sizeof(vm->r[0].v));
	rvm_asm_dump(vmcode, off);
	fprintf(stdout, "Code Execution:\n");
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
