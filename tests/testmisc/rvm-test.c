/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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
#include "rvm/rvmcpu.h"


static void test_swi_sub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	ruword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2 - op3;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static void test_swi_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	ruword res, op2 = RVM_CPUREG_GETU(cpu, R0), op3 = RVM_CPUREG_GETU(cpu, R1);

	res = op2 * op3;
	RVM_CPUREG_SETU(cpu, R0, res);
}


static rvm_switable_t switable[] = {
		{"sub", test_swi_sub},
		{"mul", test_swi_mul},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvmcpu_t *cpu;
	rvm_asmins_t code[1024];
	unsigned int off = 0;
	unsigned int ntable;

	cpu = rvm_cpu_create_default();
	ntable = rvm_cpu_addswitable(cpu, "switable", switable);

	code[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	code[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	code[off++] = rvm_asm(RVM_ADD, R2, R0, R1, 0);
	code[off++] = rvm_asm(RVM_OPSWI(RVM_SWI_ID(ntable, 1)), DA, XX, XX, 0);		// mul
	code[off++] = rvm_asm(RVM_OPSWI(RVM_SWI_ID(ntable, 0)), DA, XX, XX, 0);		// sub
	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);
	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);


	fprintf(stdout, "It works!\n");
	return 0;
}
