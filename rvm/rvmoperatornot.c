#include "rvmoperatormul.h"
#include "rvmreg.h"


static void rvm_op_not_unsigned(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1)
{
	rvmreg_t uarg;
	rword r;

	RVM_REG_SETTYPE(&uarg, RVM_DTYPE_UNSIGNED);
	rvm_opmap_invoke_binary_handler(cpu->opmap, RVM_OPID_CAST, cpu, &uarg, arg1, &uarg);
	if (cpu->error)
		return;
	r = ~(RVM_REG_GETU(&uarg));
	RVM_REG_SETU(res, r);
	RVM_STATUS_CLRALL(cpu);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


void rvm_op_not_init(rvm_opmap_t *opmap)
{
	rvm_opmap_add_unary_operator(opmap, RVM_OPID_NOT);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LSL, rvm_op_not_unsigned, RVM_DTYPE_UNSIGNED);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LSL, rvm_op_not_unsigned, RVM_DTYPE_LONG);
	rvm_opmap_set_unary_handler(opmap, RVM_OPID_LSL, rvm_op_not_unsigned, RVM_DTYPE_STRING);
}
