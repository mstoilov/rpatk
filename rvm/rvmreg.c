#include "rmem.h"
#include "rvmreg.h"
#include "rref.h"


static void rvm_reg_array_oncopy(rarray_t *array)
{
	ruint index;
	rvmreg_t *r;

	for (index = 0; index < r_array_length(array); index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		rvm_reg_copy(r, r);
	}
}


static void rvm_reg_array_oncleanup(rarray_t *array)
{
	ruint index;
	rvmreg_t *r;

	for (index = 0; index < r_array_length(array); index++) {
		r = (rvmreg_t *)r_array_slot(array, index);
		rvm_reg_cleanup(r);
	}
}


static void rvm_reg_carray_oncopy(rcarray_t *array)
{
	ruint index;
	rvmreg_t *r;
	ruint len = r_carray_length(array);


	for (index = 0; index < len; index++) {
		r = (rvmreg_t *)r_carray_slot(array, index);
		rvm_reg_copy(r, r);
	}
}


static void rvm_reg_carray_oncleanup(rcarray_t *array)
{
	ruint index;
	rvmreg_t *r;
	ruint len = r_carray_length(array);


	for (index = 0; index < len; index++) {
		r = (rvmreg_t *)r_carray_slot(array, index);
		rvm_reg_cleanup(r);
	}
}


/*
 * Recursively go over array data to unref any GC managed
 * objects. We need this because when GC is destroying arrays,
 * array's oncleanup might try to destroy the data, that should really
 * be destroyed by the GC. To avoid any attempts the same data
 * to be destroyed twice we remove the references of all GC managed
 * data from the arrays and leave the destruction of such data to
 * the GC.
 */
void rvm_reg_array_unref_gcdata(robject_t *obj)
{
	ruint size;
	int i;
	rvmreg_t *r;

	if (obj->type == R_OBJECT_ARRAY) {
		rarray_t *array = (rarray_t*)obj;
		if ((size = r_array_length(array)) > 0) {
			/*
			 * set the size to 0, to prevent circular references to come back here
			 */
			r_array_setlength(array, 0);
			for (i = 0; i < size; i++) {
				r = (rvmreg_t*) r_array_slot(array, i);
				if (rvm_reg_tstflag(r, RVM_INFOBIT_ROBJECT)) {
					robject_t *p = RVM_REG_GETP(r);
					if (!r_list_empty(&p->lnk)) {
						/*
						 * if this entry is robject_t that is on GC
						 * list, it can be RVM_REG_CLEARed. It will be
						 * cleaned up by the GC.
						 */
						rvm_reg_array_unref_gcdata(p);
						RVM_REG_CLEAR(r);
					}
				}

			}
			/*
			 * Restore the size
			 */
			r_array_setlength(array, size);
		}
	} else if (obj->type == R_OBJECT_CARRAY || obj->type == R_OBJECT_HARRAY) {
		rcarray_t *array = (rcarray_t*)obj;
		if (obj->type == R_OBJECT_HARRAY)
			array = ((rharray_t*)obj)->members;
		if ((size = r_carray_length(array)) > 0) {
			/*
			 * set the size to 0, to prevent circular references to come back here
			 */
			array->alloc_size = 0;
			for (i = 0; i < size; i++) {
				r = (rvmreg_t*) r_carray_slot(array, i);
				if (rvm_reg_tstflag(r, RVM_INFOBIT_ROBJECT)) {
					robject_t *p = RVM_REG_GETP(r);
					if (!r_list_empty(&p->lnk)) {
						/*
						 * if this entry is robject_t that is on GC
						 * list, it can be RVM_REG_CLEARed. It will be
						 * cleaned up by the GC.
						 */
						rvm_reg_array_unref_gcdata(p);
						RVM_REG_CLEAR(r);
					}
				}

			}
			/*
			 * Restore the size
			 */
			array->alloc_size = size;
		}
	}
}



rarray_t *r_array_create_rvmreg()
{
	rarray_t *array = r_array_create(sizeof(rvmreg_t));
	if (array) {
		array->oncopy = rvm_reg_array_oncopy;
		array->oncleanup = rvm_reg_array_oncleanup;
	}
	return array;
}


rcarray_t *r_carray_create_rvmreg()
{
	rcarray_t *array = r_carray_create(sizeof(rvmreg_t));
	if (array) {
		array->oncopy = rvm_reg_carray_oncopy;
		array->oncleanup = rvm_reg_carray_oncleanup;
	}
	return array;
}


rharray_t *r_harray_create_rvmreg()
{
	rharray_t *harray = r_harray_create(sizeof(rvmreg_t));
	if (harray) {
		harray->members->oncopy = rvm_reg_carray_oncopy;
		harray->members->oncleanup = rvm_reg_carray_oncleanup;
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


rvmreg_t rvm_reg_create_array()
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setarray(&r, (robject_t*)r_carray_create_rvmreg());
	return r;
}


rvmreg_t rvm_reg_create_harray()
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setharray(&r, (robject_t*)r_harray_create_rvmreg());
	return r;
}


void rvm_reg_setstring(rvmreg_t *r, rstring_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_STRING);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setpair(rvmreg_t *r, ruint p1, ruint p2)
{

	RVM_REG_SETPAIR(r, p1, p2);
	RVM_REG_SETTYPE(r, RVM_DTYPE_PAIR);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setstrptr(rvmreg_t *r, rchar *s, ruint size)
{
	RVM_REG_SETSTR(r, s, size);
	RVM_REG_SETTYPE(r, RVM_DTYPE_STRPTR);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setarray(rvmreg_t *r, robject_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_ARRAY);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setjsobject(rvmreg_t *r, robject_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_JSOBJECT);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setharray(rvmreg_t *r, robject_t *ptr)
{
	RVM_REG_SETP(r, ptr);
	RVM_REG_SETTYPE(r, RVM_DTYPE_HARRAY);
	RVM_REG_SETFLAG(r, RVM_INFOBIT_ROBJECT);
}

rvmreg_t rvm_reg_create_pair(ruint p1, ruint p2)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setpair(&r, p1, p2);
	return r;
}


rvmreg_t rvm_reg_create_strptr(rchar *s, ruint size)
{
	rvmreg_t r;
	r_memset(&r, 0, sizeof(r));
	rvm_reg_setstrptr(&r, s, size);
	return r;
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
	if (rvm_reg_tstflag(reg, RVM_INFOBIT_ROBJECT)) {
		r_object_destroy((robject_t*)RVM_REG_GETP(reg));
	}
	RVM_REG_CLEAR(reg);
}


void rvm_reg_init(rvmreg_t *reg)
{
	RVM_REG_CLEAR(reg);
}


rvmreg_t *rvm_reg_copy(rvmreg_t *dst, const rvmreg_t *src)
{
	if (dst != src)
		*dst = *src;
	if (rvm_reg_tstflag(dst, RVM_INFOBIT_ROBJECT))
		dst->v.p = r_object_v_copy(dst->v.p);
	return dst;
}


void rvm_reg_settype(rvmreg_t *r, ruint type)
{
	RVM_REG_SETTYPE(r, type);
}


ruint rvm_reg_gettype(const rvmreg_t *r)
{
	rulong type = RVM_REG_GETTYPE(r);
	return type;
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


void rvm_reg_setundef(rvmreg_t *r)
{
	RVM_REG_CLEAR(r);
	RVM_REG_SETTYPE(r, RVM_DTYPE_UNDEF);
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


void rvm_reg_setboolean(rvmreg_t *r, ruint b)
{
	RVM_REG_SETU(r, b ? 1 : 0);
	RVM_REG_SETTYPE(r, RVM_DTYPE_BOOLEAN);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setdouble(rvmreg_t *r, rdouble d)
{
	RVM_REG_SETD(r, d);
	RVM_REG_SETTYPE(r, RVM_DTYPE_DOUBLE);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


void rvm_reg_setpointer(rvmreg_t *r, rpointer p)
{
	RVM_REG_SETP(r, p);
	RVM_REG_SETTYPE(r, RVM_DTYPE_POINTER);
	RVM_REG_CLRFLAG(r, RVM_INFOBIT_ROBJECT);
}


int rvm_reg_str2num(rvmreg_t *dst, const rvmreg_t *src)
{
	rchar *dptr, *lptr;
	rdouble d;
	rlong l;

	l = r_strtol(R_STRING2ANSI(RVM_REG_GETP(src)), &lptr, 10);
	if (!lptr)
		return -1;
	if (*lptr != '.') {
		rvm_reg_setlong(dst, l);
		return 0;
	}
	d = r_strtod(R_STRING2ANSI(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setdouble(dst, d);
	return 0;
}


int rvm_reg_str2long(rvmreg_t *dst, const rvmreg_t *src)
{
	rchar *dptr;
	rdouble d;

	d = r_strtod(R_STRING2ANSI(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setlong(dst, (long)d);
	return 0;
}


int rvm_reg_str2double(rvmreg_t *dst, const rvmreg_t *src)
{
	rchar *dptr;
	rdouble d;

	d = r_strtod(R_STRING2ANSI(RVM_REG_GETP(src)), &dptr);
	if (!dptr)
		return -1;
	rvm_reg_setdouble(dst, d);
	return 0;
}


rint rvm_reg_int(const rvmreg_t *src)
{
	R_ASSERT(src);
	return (int)RVM_REG_GETL(src);
}


rlong rvm_reg_long(const rvmreg_t *src)
{
	R_ASSERT(src);
	return (rlong)RVM_REG_GETL(src);
}


ruchar rvm_reg_boolean(const rvmreg_t *src)
{
	R_ASSERT(src);
	return (ruchar)(RVM_REG_GETL(src) ? 1 : 0);
}


rdouble rvm_reg_double(const rvmreg_t *src)
{
	R_ASSERT(src);
	return (rdouble)RVM_REG_GETL(src);
}


rpointer rvm_reg_pointer(const rvmreg_t *src)
{
	R_ASSERT(src);
	return (rpointer)RVM_REG_GETP(src);
}


rstring_t *rvm_reg_string(const rvmreg_t *src)
{
	R_ASSERT(src && rvm_reg_gettype(src) == RVM_DTYPE_STRING);
	return (rstring_t*)RVM_REG_GETP(src);
}


rmap_t *rvm_reg_jsobject(const rvmreg_t *src)
{
	R_ASSERT(src && rvm_reg_gettype(src) == RVM_DTYPE_JSOBJECT);
	return (rmap_t*)RVM_REG_GETP(src);
}
