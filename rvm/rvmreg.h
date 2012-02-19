/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#ifndef _RVMREG_H_
#define _RVMREG_H_

//#include "rvm/rvmcpu.h"
#include "rlib/robject.h"
#include "rlib/rarray.h"
#include "rlib/rharray.h"
#include "rlib/rcarray.h"
#include "rlib/rmap.h"
#include "rlib/rstring.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RVM_DTYPE_NONE 0
#define RVM_DTYPE_UWORD RVM_DTYPE_NONE
#define RVM_DTYPE_UNSIGNED RVM_DTYPE_NONE
#define RVM_DTYPE_SIGNED 1
#define RVM_DTYPE_POINTER 2			/* Generic pointer, it can point to any memory object */
#define RVM_DTYPE_BOOLEAN 3
#define RVM_DTYPE_DOUBLE 4
#define RVM_DTYPE_PAIR 5
#define RVM_DTYPE_UNDEF 6
#define RVM_DTYPE_STRPTR 7
#define RVM_DTYPE_STRING 8
#define RVM_DTYPE_ARRAY 9
#define RVM_DTYPE_HARRAY 10
#define RVM_DTYPE_NAN 11
#define RVM_DTYPE_NULL RVM_DTYPE_NAN
#define RVM_DTYPE_MAP 12
#define RVM_DTYPE_FUNCTION 13
#define RVM_DTYPE_SWIID 14
#define RVM_DTYPE_OPHANDLER 15
#define RVM_DTYPE_PROPHANDLER 16
#define RVM_DTYPE_USER 17
#define RVM_DTYPE_SIZE (1 << 5)
#define RVM_DTYPE_MASK (RVM_DTYPE_SIZE - 1)
#define RVM_DTYPE_MAX (RVM_DTYPE_MASK)
#define RVM_DTYPE_USERDEF(__n__) (RVM_DTYPE_USER + (__n__))

#define RVM_INFOBIT_ROBJECT (1 << 0)
#define RVM_INFOBIT_GLOBAL (1 << 1)
#define RVM_INFOBIT_LAST (1 << 15)
#define RVM_INFOBIT_ALL (RVM_INFOBIT_ROBJECT | RVM_INFOBIT_LAST)


#define RVM_CPUREG_R_PTR(__cpu__, __r__) (&(__cpu__)->r[(__r__)])
#define RVM_CPUREG_PTR(__cpu__, __r__) RVM_CPUREG_R_PTR(__cpu__, __r__)
#define RVM_CPUREG_GET(__cpu__, __r__) *(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_SET(__cpu__, __r__, __val__) do { *(RVM_CPUREG_PTR(__cpu__, __r__)) = (__val__); } while (0)

#define RVM_REG_GETTYPE(__r__) (__r__)->type
#define RVM_REG_SETTYPE(__r__, __val__) do { (__r__)->type = (__val__); } while(0);
#define RVM_CPUREG_GETTYPE(__cpu__, __r__) RVM_REG_GETTYPE(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_SETTYPE(__cpu__, __r__, __val__) RVM_REG_SETTYPE(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_TSTFLAG(__r__, __flag__) ((__r__)->flags & (__flag__)) ? TRUE : FALSE
#define RVM_REG_SETFLAG(__r__, __flag__) do { (__r__)->flags |= (__flag__); } while (0)
#define RVM_REG_GETFLAGS(__r__) (__r__)->flags
#define RVM_REG_CLRFLAG(__r__, __flag__) do { (__r__)->flags &= ~(__flag__); } while (0)
#define RVM_REG_ASSIGNFLAGS(__r__, __flags__) do { (__r__)->flags = (__flags__); } while (0)
#define RVM_CPUREG_TSTFLAG(__cpu__, __r__, __flag__) RVM_REG_TSTFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_SETFLAG(__cpu__, __r__, __flag__) RVM_REG_SETFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_GETFLAGS(__cpu__, __r__) RVM_REG_GETFLAGS(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_CLRFLAG(__cpu__, __r__, __flag__) RVM_REG_CLRFLAG(RVM_CPUREG_PTR(__cpu__, __r__), __flag__)
#define RVM_CPUREG_ASSIGNFLAGS(__cpu__, __r__, __flags__) RVM_REG_ASSIGNFLAGS(RVM_CPUREG_PTR(__cpu__, __r__), __flags__)


#define RVM_REG_GETU(__r__) (__r__)->v.w
#define RVM_REG_SETU(__r__, __val__) do { (__r__)->v.w = (ruword)(__val__); } while (0)
#define RVM_CPUREG_GETU(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.w
#define RVM_CPUREG_SETU(__cpu__, __r__, __val__) RVM_REG_SETU(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETL(__r__) (__r__)->v.l
#define RVM_REG_SETL(__r__, __val__) do { (__r__)->v.l = (rword)(__val__); } while (0)
#define RVM_CPUREG_GETL(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.l
#define RVM_CPUREG_SETL(__cpu__, __r__, __val__) RVM_REG_SETL(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETP(__r__) (__r__)->v.p
#define RVM_REG_SETP(__r__, __val__) do { (__r__)->v.p = (rpointer)(__val__); } while (0)
#define RVM_CPUREG_GETP(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.p
#define RVM_CPUREG_SETP(__cpu__, __r__, __val__) RVM_REG_SETP(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETD(__r__) (__r__)->v.d
#define RVM_REG_SETD(__r__, __val__) do { (__r__)->v.d = (double)(__val__); } while (0)
#define RVM_CPUREG_GETD(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.d
#define RVM_CPUREG_SETD(__cpu__, __r__, __val__) RVM_REG_SETD(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETU64(__r__) (__r__)->v.u64
#define RVM_REG_SETU64(__r__, __val__) do { (__r__)->v.u64 = (ruint64)(__val__); } while (0)
#define RVM_CPUREG_GETU64(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.u64
#define RVM_CPUREG_SETU64(__cpu__, __r__, __val__) RVM_REG_SETU64(RVM_CPUREG_PTR(__cpu__, __r__), __val__)

#define RVM_REG_GETPAIR(__r__) (__r__)->v.pair
#define RVM_REG_SETPAIR(__r__, __val1__, __val2__) do { (__r__)->v.pair.p1 = (__val1__); (__r__)->v.pair.p2 = (__val2__);} while (0)
#define RVM_CPUREG_GETPAIR(__cpu__, __r__) RVM_CPUREG_PTR(__cpu__, __r__)->v.pair
#define RVM_CPUREG_SETPAIR(__cpu__, __r__, __val1__, __val2__) RVM_REG_SETPAIR(RVM_CPUREG_PTR(__cpu__, __r__), __val1__, __val2__)

#define RVM_REG_GETSTR(__r__) (__r__)->v.s
#define RVM_REG_SETSTR(__r__, __str__, __size__) do { (__r__)->v.s = (__str__); if ((__size__) == (unsigned int)-1) (__r__)->size = r_strlen(__str__); else (__r__)->size = (__size__);} while (0)
#define RVM_CPUREG_GETSTR(__cpu__, __r__) RVM_REG_GETSTR(RVM_CPUREG_PTR(__cpu__, __r__))
#define RVM_CPUREG_SETSTR(__cpu__, __r__, __str__, __size__) RVM_REG_SETSTR(RVM_CPUREG_PTR(__cpu__, __r__), __str__, __size__)


#define RVM_REG_GETIP(__r__) (rvm_asmins_t*)((__r__)->v.p)
#define RVM_REG_SETIP(__r__, __val__) do { (__r__)->v.p = (rpointer)(__val__); } while (0)
#define RVM_REG_INCIP(__r__, __val__) do {rvm_asmins_t *p = RVM_REG_GETIP(__r__); (__r__)->v.p = (rpointer)(p + (__val__)); } while (0)
#define RVM_CPUREG_GETIP(__cpu__, __r__) ((rvm_asmins_t*)RVM_CPUREG_PTR(__cpu__, __r__)->v.p)
#define RVM_CPUREG_SETIP(__cpu__, __r__, __val__) RVM_REG_SETIP(RVM_CPUREG_PTR(__cpu__, __r__), __val__)
#define RVM_CPUREG_INCIP(__cpu__, __r__, __val__) do {rvm_asmins_t *p = RVM_CPUREG_GETIP(__cpu__, __r__); (RVM_CPUREG_PTR(__cpu__, __r__))->v.p = (rpointer)(p + (__val__)); } while (0)

#define RVM_REG_GETSIZE(__r__) (__r__)->size
#define RVM_CPUREG_GETSIZE(__cpu__, __r__) RVM_REG_GETSIZE(RVM_CPUREG_PTR(__cpu__, __r__))

#define RVM_REG_CLEAR(__r__) do { (__r__)->v.w = 0UL; (__r__)->type = 0; (__r__)->flags = 0;  } while (0)
#define RVM_CPUREG_CLEAR(__cpu__, __r__) RVM_REG_CLEAR(RVM_CPUREG_PTR(__cpu__, __r__))



#define RVM_MIN_REGSIZE (sizeof(ruword))

typedef ruint16 rvmreg_type_t;
typedef ruint16 rvmreg_flags_t;

typedef struct rvmreg_s {
	union {
		ruint64 u64;
		ruword w;
		rword l;
		rpointer p;
		double d;
		char *s;
		rpair_t pair;
		ruint8 c[RVM_MIN_REGSIZE];
	} v;
	ruint32 size;
	rvmreg_type_t type;
	rvmreg_flags_t flags;
} rvmreg_t;

rvmreg_t rvm_reg_create_swi(ruword swiid);
rvmreg_t rvm_reg_create_string_ansi(const char *s);
rvmreg_t rvm_reg_create_string(const rstr_t *s);
rvmreg_t rvm_reg_create_array();
rvmreg_t rvm_reg_create_harray();
rvmreg_t rvm_reg_create_double(double d);
rvmreg_t rvm_reg_create_signed(rword l);
rvmreg_t rvm_reg_create_pair(ruint32 p1, ruint32 p2);
rvmreg_t rvm_reg_create_strptr(char *s, unsigned int size);
rvmreg_t rvm_reg_create_pointer(rpointer p);
rvmreg_t rvm_reg_create_ophandler(rpointer p);
rvmreg_t rvm_reg_create_prophandler(rpointer p);
rvmreg_type_t rvm_reg_gettype(const rvmreg_t *r);
rboolean rvm_reg_tstflag(const rvmreg_t *r, ruint16 flag);
void rvm_reg_init(rvmreg_t *reg);
void rvm_reg_cleanup(rvmreg_t *reg);
void rvm_reg_array_unref_gcdata(robject_t *obj);
void rvm_reg_settype(rvmreg_t *r, unsigned int type);
void rvm_reg_setflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_clrflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_setundef(rvmreg_t *r);
void rvm_reg_setunsigned(rvmreg_t *r, ruword u);
void rvm_reg_setboolean(rvmreg_t *r, rboolean b);
void rvm_reg_setsigned(rvmreg_t *r, rword l);
void rvm_reg_setdouble(rvmreg_t *r, double d);
void rvm_reg_setpointer(rvmreg_t *r, rpointer p);
void rvm_reg_setophandler(rvmreg_t *r, rpointer p);
void rvm_reg_setprophandler(rvmreg_t *r, rpointer p);
void rvm_reg_setpair(rvmreg_t *r, ruint32 p1, ruint32 p2);
void rvm_reg_setstrptr(rvmreg_t *r, char *s, unsigned int size);
void rvm_reg_setstring(rvmreg_t *r, rstring_t *ptr);
void rvm_reg_setarray(rvmreg_t *r, robject_t *ptr);
void rvm_reg_setharray(rvmreg_t *r, robject_t *ptr);
void rvm_reg_setjsobject(rvmreg_t *r, robject_t *ptr);
void rvm_reg_setmap(rvmreg_t *r, robject_t *ptr);
void rvm_reg_setswi(rvmreg_t *r, ruword swiid);
int rvm_reg_str2num(rvmreg_t *dst, const rvmreg_t *src);
int rvm_reg_str2signed(rvmreg_t *dst, const rvmreg_t *ssrc);
int rvm_reg_str2double(rvmreg_t *dst, const rvmreg_t *ssrc);

int rvm_reg_int(const rvmreg_t *src);
rword rvm_reg_signed(const rvmreg_t *src);
rboolean rvm_reg_boolean(const rvmreg_t *src);
double rvm_reg_double(const rvmreg_t *src);
rpointer rvm_reg_pointer(const rvmreg_t *src);
rstring_t *rvm_reg_string(const rvmreg_t *src);
rmap_t *rvm_reg_jsobject(const rvmreg_t *src);


#ifdef __cplusplus
}
#endif

#endif
