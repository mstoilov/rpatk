#include "rvm/rvmoperator.h"
#include "rvm/rvmreg.h"


void rvm_op_mul_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	rword r;

	r = op1 * op2;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_UNSIGNED);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 && (r / op1) != op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


void rvm_op_mul_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	rlong r;

	r = op1 * op2;
	RVM_REG_SETL(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_LONG);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
}


void rvm_op_mul_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	rdouble r;

	r = op1 * op2;
	RVM_REG_SETD(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_DOUBLE);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r < 0.0);
}

