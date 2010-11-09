#include <stdio.h>
#include <stdlib.h>
#include "rvmcpu.h"
#include "rvmreg.h"
#include "rvmoperator.h"
#include "rvmoperatoradd.h"
#include "rvmoperatorsub.h"
#include "rvmoperatormul.h"
#include "rvmoperatordiv.h"


typedef struct rvm_testctx_s {
	rvm_opmap_t *opmap;
} rvm_testctx_t;


static void test_swi_print_r(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_UNSIGNED)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_CPUREG_GETL(cpu, ins->op1));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_LONG)
		fprintf(stdout, "R%d = %ld\n", ins->op1, RVM_CPUREG_GETL(cpu, ins->op1));
	else if (rvm_reg_gettype(RVM_CPUREG_PTR(cpu, ins->op1)) == RVM_DTYPE_DOUBLE)
		fprintf(stdout, "R%d = %5.2f\n", ins->op1, RVM_CPUREG_GETD(cpu, ins->op1));
	else
		fprintf(stdout, "Unknown type\n");
}


static void test_swi_add(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_ADD, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), RVM_CPUREG_PTR(cpu, ins->op3));
}


static void test_swi_sub(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_SUB, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), RVM_CPUREG_PTR(cpu, ins->op3));
}


static void test_swi_mul(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_MUL, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), RVM_CPUREG_PTR(cpu, ins->op3));
}


static void test_swi_div(rvmcpu_t *cpu, rvm_asmins_t *ins)
{
	rvm_testctx_t *ctx = (rvm_testctx_t *)cpu->userdata;
	rvm_opmap_invoke_binary_handler(ctx->opmap, RVM_OPID_DIV, cpu, RVM_CPUREG_PTR(cpu, ins->op1), RVM_CPUREG_PTR(cpu, ins->op2), RVM_CPUREG_PTR(cpu, ins->op3));
}


static rvm_switable_t switable[] = {
		{"add", test_swi_add},
		{"sub", test_swi_sub},
		{"mul", test_swi_mul},
		{"div", test_swi_div},
		{"print", test_swi_print_r},
		{NULL, NULL},
};


int main(int argc, char *argv[])
{
	rvm_testctx_t ctx;
	rvmcpu_t *cpu;
	rvm_opmap_t *opmap;
	rvm_asmins_t code[1024];
	ruint off = 0;
	ruint ntable;

	ctx.opmap = opmap = rvm_opmap_create();
	cpu = rvm_cpu_create();
	cpu->userdata = &ctx;

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_ADD);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_ADD, rvm_op_add_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_SUB);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_SUB, rvm_op_sub_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_MUL);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_MUL, rvm_op_mul_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);

	rvm_opmap_add_binary_operator(opmap, RVM_OPID_DIV);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_double_double, RVM_DTYPE_DOUBLE, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_long_double, RVM_DTYPE_UNSIGNED, RVM_DTYPE_DOUBLE);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_double_long, RVM_DTYPE_DOUBLE, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_binary_handler(opmap, RVM_OPID_DIV, rvm_op_div_long_long, RVM_DTYPE_UNSIGNED, RVM_DTYPE_UNSIGNED);

	ntable = rvmcpu_switable_add(cpu, switable);
	code[off++] = rvm_asm(RVM_MOV, R1, DA, XX, 1);
	code[off++] = rvm_asm(RVM_MOV, R2, DA, XX, 3);
	code[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi(cpu, "add")), R0, R1, R2, 0);
	code[off++] = rvm_asm(RVM_MOV, R3, R0, XX, 0);
	code[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi(cpu, "print")), R1, XX, XX, 0);	// print
	code[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi(cpu, "print")), R2, XX, XX, 0);	// print
	code[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi(cpu, "print")), R0, XX, XX, 0);	// print
	code[off++] = rvm_asml(RVM_OPSWI(rvm_cpu_getswi(cpu, "mul")), R0, R3, DA, 3);
	code[off++] = rvm_asm(RVM_OPSWI(rvm_cpu_getswi(cpu, "print")), R0, XX, XX, 0);	// print

	code[off++] = rvm_asm(RVM_EXT, XX, XX, XX, 0);



	rvm_cpu_exec_debug(cpu, code, 0);
	rvm_cpu_destroy(cpu);
	rvm_opmap_destroy(opmap);


	fprintf(stdout, "It works!\n");
	return 0;
}
