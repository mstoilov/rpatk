#include <stdio.h>
#include <stdlib.h>
#include "rvm/rvmcpu.h"
#include "rvm/rvmreg.h"
#include "rvm/rvmoperator.h"


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_asmins_t code[1024];
	ruinteger off = 0;

	cpu = rvm_cpu_create_default();


	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);

	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);


	fprintf(stdout, "It works!\n");
	return 0;
}
