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
	int table1, table2, table3;
	unsigned int ret = 0;
	unsigned int off = 0;
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
	rvm_asm_dump(vm, vmcode, off);
	fprintf(stdout, "Code Execution:\n");
#ifdef EXECDEBUG
	ret = rvm_cpu_exec_debug(vm, vmcode, 0);
#else
	ret = rvm_cpu_exec(vm, vmcode, 0);
#endif
	
	rvm_cpu_destroy(vm);
	return 0;
}
