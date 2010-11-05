#include "rvmoperatorsub.h"
#include "rvmreg.h"


void rvm_op_sub_long_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rlong r = (rlong)arg1->v.l - (rlong)arg2->v.l;
	res->v.l = r;
	rvm_reg_settype(res, RVM_DTYPE_LONG);
}


void rvm_op_sub_double_long(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = arg1->v.d - (rdouble)arg2->v.l;
	res->v.d = r;
	rvm_reg_settype(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_sub_long_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rlong r = (rlong)arg1->v.l - (rlong)arg2->v.d;
	res->v.l = r;
	rvm_reg_settype(res, RVM_DTYPE_LONG);
}


void rvm_op_sub_double_double(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rdouble r = arg1->v.d - arg2->v.d;
	res->v.d = r;
	rvm_reg_settype(res, RVM_DTYPE_DOUBLE);
}
