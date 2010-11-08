#ifndef _RVMREG_H_
#define _RVMREG_H_

#include "rvmcpu.h"
#include "robject.h"
#include "rarray.h"
#include "rharray.h"
#include "rstring.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Create array with rvmreg elements */
rarray_t *r_array_create_rvmreg();
/* Create harray with rvmreg elements */
rharray_t *r_harray_create_rvmreg();

rvmreg_t rvm_reg_create_string_ansi(const rchar *s);
rvmreg_t rvm_reg_create_string(const rstr_t *s);
rvmreg_t rvm_reg_create_array();
rvmreg_t rvm_reg_create_harray();
rvmreg_t rvm_reg_create_refreg();
rvmreg_t rvm_reg_create_double(rdouble d);
rvmreg_t rvm_reg_create_long(rlong l);
void rvm_reg_cleanup(rvmreg_t *reg);
rvmreg_t *rvm_reg_copy(rvmreg_t *dst, const rvmreg_t *src);
rvmreg_t *rvm_reg_refer(rvmreg_t *dst, const rvmreg_t *src);
void rvm_reg_settype(rvmreg_t *r, ruint type);
ruint rvm_reg_gettype(const rvmreg_t *r);
rboolean rvm_reg_tstflag(const rvmreg_t *r, ruint16 flag);
void rvm_reg_setflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_clrflag(rvmreg_t *r, ruint16 flag);
void rvm_reg_setlong(rvmreg_t *r, rlong l);
void rvm_reg_setdouble(rvmreg_t *r, rdouble d);
void rvm_reg_setstring(rvmreg_t *r, rstring_t *ptr);
void rvm_reg_setarray(rvmreg_t *r, rarray_t *ptr);
void rvm_reg_setharray(rvmreg_t *r, rharray_t *ptr);
void rvm_reg_convert_to_refreg(rvmreg_t *r);
rvmreg_t *rvm_reg_unshadow(rvmreg_t *reg);

struct rrefreg_s;
void rvm_reg_setrefreg(rvmreg_t *r, struct rrefreg_s *ptr);


#ifdef __cplusplus
}
#endif

#endif
