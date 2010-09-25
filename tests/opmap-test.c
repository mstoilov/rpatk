#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatoradd.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_add(rvm_cpu_t *cpu)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_handler(ctx->opmap, RVM_OPID_ADD, cpu, &cpu->r[R0], &cpu->r[R0], &cpu->r[R1]);
}


static void test_swi_mul(rvm_cpu_t *cpu)
{
	rword res, op2 = RVM_GET_REGU(cpu, R0), op3 = RVM_GET_REGU(cpu, R1);

	res = op2 * op3;
	RVM_SET_REGU(cpu, R0, res);
}


static rvm_switable_t switable[] = {
		{"add", test_swi_add},
		{"mul", test_swi_mul},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvm_testctx_t ctx;
	rvm_cpu_t *cpu;
	rvm_opmap_t *opmap;
	rvm_asmins_t code[1024];
	ruint off = 0;
	ruint ntable;

	ctx.opmap = opmap = rvm_opmap_create();
	cpu = rvm_cpu_create();
	cpu->userdata = &ctx;

	rvm_opmap_add_operator(opmap, RVM_OPID_ADD);
	rvm_opmap_set_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);


	ntable = rvm_cpu_switable_add(cpu, switable);
	ntable = rvm_cpu_switable_add(cpu, switable);
	code[off++] = rvm_asm(RVM_MOV, R0, DA, XX, 1);
	code[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 2);
	code[off++] = rvm_asm(RVM_ADD, R2, R0, R1, 0);
	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, RVM_SWI_ID(ntable, 1));		// mul
	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, RVM_SWI_ID(ntable, 0));		// add
	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);



	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);


	fprintf(stdout, "It works!\n");
	return 0;
}
