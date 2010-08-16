#include <stdio.h>
#include "common.h"


static void rvm_callback_one(rvm_cpu_t *vm)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}



static rvm_switable_t calltable[] = {
	{"rvm_callback_one", rvm_callback_one},
	{NULL, NULL},
};
	
	
	
int main(int argc, char *argv[])
{
	rint table1, table2, table3;
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvm_cpu_t *vm = rvm_cpu_create();

	table1 = rvm_cpu_switable_add(vm, calltable);
	table2 = rvm_cpu_switable_add(vm, calltable);
	table3 = rvm_cpu_switable_add(vm, calltable);
	if (table2 != -1 || table3 != -1) {
		fprintf(stdout, "rvm_cpu_switable_add: FAILED\n");
	}
	rvm_cpu_switable_add(vm, common_calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_SWI, DA, XX, XX, rvm_cpu_getswi(vm, "rvm_callback_one"));
	vmcode[off++] = rvm_asm(RVM_SWI, DA, XX, XX, rvm_cpu_getswi(vm, "rvm_callback_one"));
	vmcode[off++] = rvm_asm(RVM_SWI, DA, XX, XX, RVM_SWI_ID(table1, 0));
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
	fprintf(stdout, "Code List (sizeof rvm_reg_t is: %d(%d)):\n", (unsigned int) sizeof(rvm_reg_t), (unsigned int) sizeof(vm->r[0].v));
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
