/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "rvm/rvmoperator.h"


static void rvm_eadd(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvmreg_t *arg2 = RVM_CPUREG_PTR(cpu, ins->op2);
	rvmreg_t *arg3 = RVM_CPUREG_PTR(cpu, ins->op3);

	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_ADD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), arg2, arg3);
//	fprintf(stdout, "%s %ld\n", __FUNCTION__, RVM_CPUREG_GETU(cpu, ins->op1));
}


static void rvm_callback_two(rvmcpu_t *vm, rvm_asmins_t *ins)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
}


static rvm_switable_t calltable[] = {
	{"rvm_callback_two", rvm_callback_two},
	{"rvm_eadd", rvm_eadd},
	{NULL, NULL},
};

	
int main(int argc, char *argv[])
{
	long iter = 1;
	unsigned int ret = 0;
	unsigned int off = 0;
	rvm_asmins_t vmcode[256];
	rvmcpu_t *vm = rvm_cpu_create_default();

	if (argc > 1) {
		iter = atol(argv[1]);
	}

	rvm_cpu_addswitable(vm, "calltable", calltable);
	rvm_cpu_addswitable(vm, "common_table", common_calltable);

	vmcode[off++] = rvm_asm(RVM_MOV, R4, DA, XX, 0);
	vmcode[off++] = rvm_asm(RVM_MOV, R5, DA, XX, iter);
	vmcode[off++] = rvm_asmd(RVM_EMUL, R4, R4, DA, 2.0);
	vmcode[off++] = rvm_asmd(RVM_EDIV, R4, R4, DA, 2.0);
	vmcode[off++] = rvm_asm(RVM_EADD, R4, R4, DA, 2);
	vmcode[off++] = rvm_asm(RVM_ESUB, R4, R4, DA, 1);
//	vmcode[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
//	vmcode[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 2);
//	vmcode[off++] = rvm_asm(RVM_EADD, R0, R1, R2, 0);
//	vmcode[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_swilookup(vm, "rvm_eadd")), R0, R1, R2, 0);
//	vmcode[off++] = rvm_asm(RVM_PUSH, DA, XX, XX, 1);
//	vmcode[off++] = rvm_asm(RVM_POP, R1, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_PUSH, R4, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_POP, R4, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_ECMP, R4, R5, XX, 0);
	vmcode[off++] = rvm_asml(RVM_BLES, DA, XX, XX, -7);
	vmcode[off++] = rvm_asm(RVM_MOV, R0, R4, XX, 0);
	vmcode[off++] = rvm_asm(RVM_PRN, R0, XX, XX, 0);
	vmcode[off++] = rvm_asm(RVM_EXT, R0, XX, XX, 0);

	ret = rvm_cpu_exec(vm, vmcode, 0);
	fprintf(stdout, "R0 = %ld (%ld operations)\n", (unsigned long) RVM_CPUREG_GETU(vm, R0), (unsigned long)RVM_CPUREG_GETU(vm, R5));
	rvm_cpu_destroy(vm);
	return 0;
}
