#include <stdio.h>
#include "rpavm.h"


static ruint rvm_callback_one(rvm_cpu_t *vm)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
	return 0;
}



static rvm_cpu_swi calltable[] = {
	rvm_callback_one,
	NULL,
};
	
	
	
int main(int argc, char *argv[])
{
	int table1, table2, table3;
	ruint ret = 0;
	ruint off = 0;
	rvm_asmins_t vmcode[256];
	rvm_cpu_t *vm = rvm_cpu_create();

	table1 = rvm_cpu_switable_add(vm, calltable);
	table2 = rvm_cpu_switable_add(vm, calltable);
	table3 = rvm_cpu_switable_add(vm, calltable);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	vmcode[off++] = rvm_asm(RVM_ADD, R0, R1, R0, 0);
	vmcode[off++] = rvm_asm(RVM_EXCALL, DA, XX, XX, RVM_CB(table1, 0));
	vmcode[off++] = rvm_asm(RVM_EXCALL, DA, XX, XX, RVM_CB(table2, 0));
	vmcode[off++] = rvm_asm(RVM_EXCALL, DA, XX, XX, RVM_CB(table3, 0));
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);
	rvm_asm_dump(vmcode, off);
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
