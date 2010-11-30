#include "rmem.h"
#include "rvmreg.h"
#include "rrefreg.h"
#include "rref.h"


static void r_array_oncopy_rvmreg(rarray_t *array)
{
	ruint index;
	rvmreg_t *r;

	for (index = 0; index < array->len; index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		rvm_reg_copy(r, r);
	}
}


static void r_array_ondestroy_rvmreg(rarray_t *array)
{
	ruint index;
	rvmreg_t *r;

	for (index = 0; index < array->len; index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		rvm_reg_cleanup(r);
	}
}


rarray_t *r_array_create_rvmreg()
{
	rarray_t *array = r_array_create(sizeof(rvmreg_t));
	if (array) {
		array->oncopy = r_array_oncopy_rvmreg;
		array->ondestroy = r_array_ondestroy_rvmreg;
	}
	return array;
}


rharray_t *r_harray_create_rvmreg()
{
	rharray_t *harray = r_harray_create(sizeof(rvmreg_t));
	if (harray) {
		harray->members->oncopy = r_array_oncopy_rvmreg;
		harray->members->ondestroy = r_array_ondestroy_rvmreg;
	}
	return harray;
}


rvmreg_t rvm_reg_create_string_ansi(const rchar *s)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	RVM_REG_SETP(&r, r_string_create_from_ansistr(s));
	RVM_REG_SETTYPE(&r, RVM_DTYPE_STRING);
	RVM_REG_SETFLAG(&r, RVM_INFOBIT_ROBJECT);
	return r;
}


rvmreg_t rvm_reg_create_string(const rstr_t *s)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	RVM_REG_SETP(&r, r_string_create_from_rstr(s));
	RVM_REG_SETTYPE(&r, RVM_DTYPE_STRING);
	RVM_REG_SETFLAG(&r, RVM_INFOBIT_ROBJECT);
	return r;
}

rvmreg_t rvm_reg_create_refreg()
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	RVM_REG_SETP(&r, r_refreg_create());
	RVM_REG_SETTYPE(&r, RVM_DTYPE_REFREG);
	RVM_REG_SETFLAG(&r, RVM_INFOBIT_ROBJECT);
	return r;
}


rvmreg_t rvm_reg_create_array()
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setarray(&r, r_array_create_rvmreg());
	return r;
}


rvmreg_t rvm_reg_create_harray()
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setharray(&r, r_harray_create_rvmreg());
	return r;
}

void rvm_reg_setstring(rvmreg_t *r, rstring_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_STRING);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setarray(rvmreg_t *r, rarray_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_ARRAY);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setharray(rvmreg_t *r, rharray_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_HARRAY);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


rvmreg_t rvm_reg_create_double(rdouble d)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	RVM_REG_SETD(&r, d);
	RVM_REG_SETTYPE(&r, RVM_DTYPE_DOUBLE);
	return r;
}


rvmreg_t rvm_reg_create_long(rlong l)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setlong(&r, l);
	return r;
}


void rvm_reg_cleanup(rvmreg_t *reg)
{
	if (rvm_reg_gettype(reg) == RVM_DTYPE_REFREG)
		r_ref_dec((rref_t*)RVM_REG_GETP(reg));
	else if (rvm_reg_tstflag(reg, RVM_INFOBIT_ROBJECT)) {
		r_object_destroy((robject_t*)RVM_REG_GETP(reg));
	}
	RVM_REG_CLEAR(reg);
}


rvmreg_t *rvm_reg_copy(rvmreg_t *dst, const rvmreg_t *src)
{
	if (dst != src)
		*dst = *src;
	if (rvm_reg_tstflag(dst, RVM_INFOBIT_ROBJECT))
		dst->v.p = r_object_copy(dst->v.p);
	return dst;
}


rvmreg_t *rvm_reg_refer(rvmreg_t *dst, const rvmreg_t *src)
{
	if (rvm_reg_gettype(dst) == RVM_DTYPE_REFREG) {
		if (dst != src)
			*dst = *src;
		r_ref_inc((rref_t*)RVM_REG_GETP(dst));
		return dst;
	}
	return NULL;
}


void rvm_reg_settype(rvmreg_t *r, ruint type)
{
	RVM_REG_SETTYPE(r, type);
}


ruint rvm_reg_gettype(const rvmreg_t *r)
{
	return RVM_REG_GETTYPE(r);
}


rboolean rvm_reg_tstflag(const rvmreg_t *r, ruint16 flag)
{
	return RVM_REG_TSTFLAG(r, flag);
}


void rvm_reg_setflag(rvmreg_t *r, ruint16 flag)
{
	RVM_REG_SETFLAG(r, flag);
}


void rvm_reg_clrflag(rvmreg_t *r, ruint16 flag)
{
	RVM_REG_CLRFLAG(r, flag);
}


void rvm_reg_setunsigned(rvmreg_t *r, rword u)
{
	RVM_REG_SETU(r, u);
	RVM_REG_SETTYPE(r, RVM_DTYPE_UNSIGNED);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setlong(rvmreg_t *r, rlong l)
{
	RVM_REG_SETL(r, l);
	RVM_REG_SETTYPE(r, RVM_DTYPE_LONG);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setdouble(rvmreg_t *r, rdouble d)
{
	RVM_REG_SETD(r, d);
	RVM_REG_SETTYPE(r, RVM_DTYPE_DOUBLE);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);

}


void rvm_reg_setrefreg(rvmreg_t *r, struct rrefreg_s *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_REFREG);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_convert_to_refreg(rvmreg_t *reg)
{
	rrefreg_t * refreg = NULL;

	if (rvm_reg_gettype(reg) == RVM_DTYPE_REFREG)
		return;
	refreg = r_refreg_create();
	*REFREG2REGPTR(refreg) = *reg;
	RVM_REG_CLEAR(reg);
	rvm_reg_setrefreg(reg, refreg);
}


rvmreg_t *rvm_reg_unshadow(const rvmreg_t *reg)
{
	if (rvm_reg_gettype(reg) != RVM_DTYPE_REFREG)
		return (rvmreg_t*)reg;
	return REFREG2REGPTR(RVM_REG_GETP(reg));
}


int rvm_reg_str2num(rvmreg_t *dst, const rvmreg_t *ssrc)
{
	rchar *dptr, *lptr;
	rdouble d;
	rlong l;
	const rvmreg_t *src = rvm_reg_unshadow(ssrc);

	l = r_strtol(R_STRING2PTR(RVM_REG_GETP(src)), &lptr, 10);
	if (!lptr)
		return -1;
	if (*lptr != '.') {
		rvm_reg_setlong(dst, l);
		return 0;
	}
	d = r_strtod(R_STRING2PTR(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setdouble(dst, d);
	return 0;
}


int rvm_reg_str2long(rvmreg_t *dst, const rvmreg_t *ssrc)
{
	rchar *dptr;
	rdouble d;
	const rvmreg_t *src = rvm_reg_unshadow(ssrc);

	d = r_strtod(R_STRING2PTR(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setlong(dst, (long)d);
	return 0;
}


int rvm_reg_str2double(rvmreg_t *dst, const rvmreg_t *ssrc)
{
	rchar *dptr;
	rdouble d;
	const rvmreg_t *src = rvm_reg_unshadow(ssrc);

	d = r_strtod(R_STRING2PTR(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setdouble(dst, d);
	return 0;
}
