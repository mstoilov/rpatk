#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rvmoperator.h"
#include "rvmoperatoradd.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_print_r0(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_gettype(&cpu->r[R0]) == RVM_DTYPE_LONG)
		fprintf(stdout, "R0 = %ld\n", RVM_GET_REGL(cpu, R0));
	else if (rvm_reg_gettype(&cpu->r[R0]) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R0 = %5.2f\n", RVM_GET_REGD(cpu, R0));
	else
		fprintf(stdout, "Unknown type\n");
}


static void test_swi_add(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_ADD, cpu, &cpu->r[R0], &cpu->r[R1], &cpu->r[R2]);
}


static void test_swi_sub(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_SUB, cpu, &cpu->r[R0], &cpu->r[R1], &cpu->r[R2]);
}


static void test_swi_mul(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_MUL, cpu, &cpu->r[R0], &cpu->r[R1], &cpu->r[R2]);
}


static void test_swi_div(rvm_cpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_DIV, cpu, &cpu->r[R0], &cpu->r[R1], &cpu->r[R2]);
}


static rvm_switable_t switable[] = {
		{"add", test_swi_add},
		{"sub", test_swi_sub},
		{"mul", test_swi_mul},
		{"div", test_swi_div},
		{"print", test_swi_print_r0},
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

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_ADD);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_double, RVM_DTYPE_LONG, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_LONG);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_long, RVM_DTYPE_LONG, RVM_DTYPE_LONG);


	ntable = rvm_cpu_switable_add(cpu, switable);
	code[off++] = rvm_asmd(RVM_MOV, R1, DA, XX, 1);
	code[off++] = rvm_asml(RVM_MOV, R2, DA, XX, 3.2);
//	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, RVM_SWI_ID(ntable, 1));			// mul

	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, rvm_cpu_getswi(cpu, "add"));		// add
	code[off++] = rvm_asm(RVM_SWI, DA, XX, XX, rvm_cpu_getswi(cpu, "print"));	// print

	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);



	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);


	fprintf(stdout, "It works!\n");
	return 0;
}
