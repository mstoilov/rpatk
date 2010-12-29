#ifndef __RVMOPERATORCAST_H_
#define __RVMOPERATORCAST_H_

#include "rvmoperator.h"
#include "rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

void rvm_op_cast_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2);
void rvm_op_cast_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2);
void rvm_op_cast_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2);

void rvm_op_cast_init(rvm_opmap_t *opmap);

#ifdef __cplusplus
}
#endif

#endif
