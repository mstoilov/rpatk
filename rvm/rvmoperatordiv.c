#include "rvmoperatordiv.h"


void rvm_op_div_long_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2)
{
	rlong r;
	if (!arg2->v.l) {
		cpu->error = RVM_E_DIVZERO;
		cpu->abort = 1;
		return;
	}
	r = (rlong)arg1->v.l / (rlong)arg2->v.l;
	res->v.l = r;
	rvm_reg_settype(res, RVM_DTYPE_LONG);
}


void rvm_op_div_double_long(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2)
{
	rdouble r;
	if (!arg2->v.l) {
		cpu->error = RVM_E_DIVZERO;
		cpu->abort = 1;
		return;
	}
	r = arg1->v.d / (rdouble)arg2->v.l;
	res->v.d = r;
	rvm_reg_settype(res, RVM_DTYPE_DOUBLE);
}


void rvm_op_div_long_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2)
{
	rlong r;

	if (!arg2->v.d) {
		cpu->error = RVM_E_DIVZERO;
		cpu->abort = 1;
		return;
	}
	r = (rlong)arg1->v.l / (rlong)arg2->v.d;
	res->v.l = r;
	rvm_reg_settype(res, RVM_DTYPE_LONG);
}


void rvm_op_div_double_double(rvm_cpu_t *cpu, rvm_reg_t *res, const rvm_reg_t *arg1, const rvm_reg_t *arg2)
{
	rdouble r;

	if (!arg2->v.d) {
		cpu->error = RVM_E_DIVZERO;
		cpu->abort = 1;
		return;
	}
	r = arg1->v.d / arg2->v.d;
	res->v.d = r;
	rvm_reg_settype(res, RVM_DTYPE_DOUBLE);
}