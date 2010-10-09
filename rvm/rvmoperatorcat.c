#include "rvmoperatorcat.h"
#include "rstring.h"


void rvm_op_cat_string_string(rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2)
{
	rstring_t *s1 = (rstring_t*)arg1->v.p;
	rstring_t *s2 = (rstring_t*)arg2->v.p;
	rstring_t *dst;

	dst = r_string_create_from_rstr(&s1->s);
	r_string_cat(dst, &s2->s);
	RVM_REG_UNREF(res);
	RVM_REG_SETSTR(res, dst);
}
