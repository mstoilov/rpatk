#ifndef __RVMOPERATORBIN_H_
#define __RVMOPERATORBIN_H_

#include "rvmoperator.h"
#include "rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rvm_binop_unsigned)(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2);
typedef void (*rvm_binop_long)(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2);
typedef void (*rvm_binop_double)(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2);


typedef struct rvm_binopmap_s {
	rushort opid;
	rvm_binop_unsigned unsigned_binop_fun;
	rvm_binop_long long_binop_fun;
	rvm_binop_double double_binop_fun;
} rvm_binopmap_t;

void rvm_op_binary_insert(rvm_opmap_t *opmap, rushort opid, rvm_binop_unsigned u, rvm_binop_long l, rvm_binop_double d);
void rvm_op_binary_init(rvm_opmap_t *opmap);

#ifdef __cplusplus
}
#endif

#endif
