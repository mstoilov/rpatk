#include "rvm/rvmoperator.h"
#include "rvm/rvmreg.h"


void rvm_op_cmp_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	rword r;

	r = op1 - op2;
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r & RVM_SIGN_BIT);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_V, (op1 & RVM_SIGN_BIT) != (op2 & RVM_SIGN_BIT) &&
							(r & RVM_SIGN_BIT) == (op1 & RVM_SIGN_BIT));

}


void rvm_op_cmp_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	rvm_op_cmp_unsigned(cpu, opid, res, op1, op2);
}


void rvm_op_cmp_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	rdouble r;

	r = op1 - op2;
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_C, op1 < op2);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_N, r < 0.0);
}
