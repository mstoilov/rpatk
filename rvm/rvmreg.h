#ifndef _RVMREG_H_
#define _RVMREG_H_

//#include "rvmcpu.h"
#include "robject.h"
#include "rarray.h"
#include "rharray.h"
#include "rcarray.h"
#include "rstring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RVM_DTYPE_NONE 0
#define RVM_DTYPE_WORD RVM_DTYPE_NONE
#define RVM_DTYPE_UNSIGNED RVM_DTYPE_NONE
#define RVM_DTYPE_LONG 1
#define RVM_DTYPE_POINTER 2			/* Generic pointer, it can point to any memory object */
#define RVM_DTYPE_BOOLEAN 3
#define RVM_DTYPE_STRING 4
#define RVM_DTYPE_ARRAY 5
#define RVM_DTYPE_HARRAY 6
#define RVM_DTYPE_DOUBLE 7
#define RVM_DTYPE_RELOCPTR 14		/* Relocation, using pointers */
#define RVM_DTYPE_RELOCINDEX 15		/* Relocation, using offsets */
#define RVM_DTYPE_USER 16
#define RVM_DTYPE_SIZE (1 << 5)
#define RVM_DTYPE_MASK (RVM_DTYPE_SIZE - 1)
#define RVM_DTYPE_MAX (RVM_DTYPE_MASK)
#define RVM_DTYPE_USERDEF(__n__) (RVM_DTYPE_USER + (__n__))

#define RVM_INFOBIT_ROBJECT (1 << 0)
#define RVM_INFOBIT_LAST (1 << 15)
#define RVM_INFOBIT_ALL (RVM_INFOBIT_ROBJECT | RVM_INFOBIT_LAST)

#define r_carray_rvmregslot(__carray__, __index__)((rvmreg_t*)(((rchar*)r_array_index((__carray__)->array, (__index__) >> R_CARRAY_CHUNKBITS, rpointer)) + ((__index__) & R_CARRAY_CHUNKMASK) * sizeof(rvmreg_t)))
#define RVM_STACK_ADDR(__cpu__, __off__) ((rvmreg_t*)r_carray_rvmregslot((__cpu__)->stack, (__off__) ))
#define RVM_SPOFF_ADDR(__cpu__, __spoff__) RVM_STACK_ADDR(__cpu__, (RVM_CPUREG_GETU(__cpu__, SP) - (__spoff__)))

#define RVM_CPUREG_R_PTR(__cpu__, __r__) (&(__cpu__)->r[(__r__)])
#define RVM_CPUREG_PTR(__cpu__, __r__) RVM_CPUREG_R_PTR(__cpu__, __r__)
#define RVM_CPUREG_GET(__cpu__, __r__) *(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_SET(__cpu__, __r__, __val__) do { *(RVM_CPUREG_PTR(__cpu__, __r__)) = (rvmreg_t)(__val__); } while (0)

#define RVM_REG_GETTYPE(__r__) (__r__)->type
#define RVM_REG_SETTYPE(__r__, __val__) do { (__r__)->type = (__val__); } while(0);
#define RVM_CPUREG_GETTYPE(__cpu__, __r__) RVM_REG_GETTYPE(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_SETTYPE(__cpu__, __r__, __val__) RVM_REG_SETTYPE(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_TSTFLAG(__r__, __flag__) ((__r__)->flags & (__flag__)) ? TRUE : FALSE
#define RVM_REG_SETFLAG(__r__, __flag__) do { (__r__)->flags |= (__flag__); } while (0)
#define RVM_REG_CLRFLAG(__r__, __flag__) do { (__r__)->flags &= ~(__flag__); } while (0)
#define RVM_REG_ASSIGNFLAGS(__r__, __flags__) do { (__r__)->flags = ~(__flags__); } while (0)
#define RVM_CPUREG_TSTFLAG(__cpu__, __r__, __flag__) RVM_REG_TSTFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_SETFLAG(__cpu__, __r__, __flag__) RVM_REG_SETFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_CLRFLAG(__cpu__, __r__, __flag__) RVM_REG_CLRFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_ASSIGNFLAGS(__cpu__, __r__, __flags__) RVM_REG_ASSIGNFLAGS(RVM_CPUREG_PTR(__cpu__, __r__), __flags__)

#define RVM_REG_GETU(__r__) (__r__)->v.w
#define RVM_REG_SETU(__r__, __val__) do { (__r__)->v.w = (rword)(__val__); } while (0)
#define RVM_CPUREG_GETU(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.w
#define RVM_CPUREG_SETU(__cpu__, __r__, __val__) RVM_REG_SETU(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETL(__r__) (__r__)->v.l
#define RVM_REG_SETL(__r__, __val__) do { (__r__)->v.l = (rlong)(__val__); } while (0)
#define RVM_CPUREG_GETL(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.l
#define RVM_CPUREG_SETL(__cpu__, __r__, __val__) RVM_REG_SETL(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETP(__r__) (__r__)->v.p
#define RVM_REG_SETP(__r__, __val__) do { (__r__)->v.p = (rpointer)(__val__); } while (0)
#define RVM_CPUREG_GETP(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.p
#define RVM_CPUREG_SETP(__cpu__, __r__, __val__) RVM_REG_SETP(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETD(__r__) (__r__)->v.d
#define RVM_REG_SETD(__r__, __val__) do { (__r__)->v.d = (rdouble)(__val__); } while (0)
#define RVM_CPUREG_GETD(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.d
#define RVM_CPUREG_SETD(__cpu__, __r__, __val__) RVM_REG_SETD(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETIP(__r__) (rvm_asmins_t*)((__r__)->v.p)
#define RVM_REG_SETIP(__r__, __val__) do { (__r__)->v.p = (rpointer)(__val__); } while (0)
#define RVM_REG_INCIP(__r__, __val__) do {rvm_asmins_t *p = RVM_REG_GETIP(__r__); (__r__)->v.p = (rpointer)(p + (__val__)); } while (0)
#define RVM_CPUREG_GETIP(__cpu__, __r__) ((rvm_asmins_t*)RVM_CPUREG_PTR(__cpu__, __r__)->v.p)
#define RVM_CPUREG_SETIP(__cpu__, __r__, __val__) RVM_REG_SETIP(RVM_CPUREG_PTR(__cpu__, __r__), __val__)
#define RVM_CPUREG_INCIP(__cpu__, __r__, __val__) do {rvm_asmins_t *p = RVM_CPUREG_GETIP(__cpu__, __r__); (RVM_CPUREG_PTR(__cpu__, __r__))->v.p = (rpointer)(p + (__val__)); } while (0)

#define RVM_REG_SIZE(__r__) (__r__)->size
#define RVM_REG_CLEAR(__r__) do { (__r__)->v.w = 0UL; (__r__)->type = 0; (__r__)->flags = 0;  } while (0)
#define RVM_CPUREG_CLEAR(__cpu__, __r__) RVM_REG_CLEAR(RVM_CPUREG_PTR(__cpu__, __r__))


#define RVM_MIN_REGSIZE (sizeof(rword)/8)

typedef ruint16 rvmreg_type_t;
typedef ruint16 rvmreg_flags_t;

typedef struct rvmreg_s {
	union {
		rword w;
		rlong l;
		rpointer p;
		rdouble d;
		ruint8 c[RVM_MIN_REGSIZE];
	} v;
	rvmreg_type_t type;
	rvmreg_flags_t flags;
	ruint32 size;
} rvmreg_t;

/* Create array with rvmreg elements */
rarray_t *r_array_create_rvmreg();
/* Create harray with rvmreg elements */
rharray_t *r_harray_create_rvmreg();
/* Create carray with rvmreg elements */
rcarray_t *r_carray_create_rvmreg();

rvmreg_t rvm_reg_create_string_ansi(const rchar *s);
rvmreg_t rvm_reg_create_string(const rstr_t *s);
rvmreg_t rvm_reg_create_array();
rvmreg_t rvm_reg_create_harray();
rvmreg_t rvm_reg_create_double(rdouble d);
rvmreg_t rvm_reg_create_long(rlong l);
void rvm_reg_cleanup(rvmreg_t *reg);
rvmreg_t *rvm_reg_copy(rvmreg_t *dst, const rvmreg_t *src);
void rvm_reg_array_unref_gcdata(robject_t *obj);
void rvm_reg_settype(rvmreg_t *r, ruint type);
ruint rvm_reg_gettype(const rvmreg_t *r);
rboolean rvm_reg_tstflag(const rvmreg_t *r, ruint16 flag);
void rvm_reg_setflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_clrflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_setunsigned(rvmreg_t *r, rword u);
void rvm_reg_setlong(rvmreg_t *r, rlong l);
void rvm_reg_setdouble(rvmreg_t *r, rdouble d);
void rvm_reg_setstring(rvmreg_t *r, rstring_t *ptr);
void rvm_reg_setarray(rvmreg_t *r, robject_t *ptr);
void rvm_reg_setharray(rvmreg_t *r, robject_t *ptr);
int rvm_reg_str2num(rvmreg_t *dst, const rvmreg_t *src);
int rvm_reg_str2long(rvmreg_t *dst, const rvmreg_t *ssrc);
int rvm_reg_str2double(rvmreg_t *dst, const rvmreg_t *ssrc);


#ifdef __cplusplus
}
#endif

#endif
