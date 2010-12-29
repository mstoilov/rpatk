#include "rvmoperator.h"
#include "rvmreg.h"


void rvm_op_lesseq_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2)
{
	rword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


void rvm_op_lesseq_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2)
{
	rword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}


void rvm_op_lesseq_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2)
{
	rword r;

	r = (op1 <= op2) ? 1 : 0;
	RVM_REG_SETU(res, r);
	RVM_REG_SETTYPE(res, RVM_DTYPE_BOOLEAN);
	RVM_STATUS_UPDATE(cpu, RVM_STATUS_Z, !r);
}
