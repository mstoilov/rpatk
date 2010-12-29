#ifndef __RVMOPERATORCAT_H_
#define __RVMOPERATORCAT_H_

#include "rvmoperator.h"
#include "rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

void rvm_op_cat_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2);
void rvm_op_cat_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2);
void rvm_op_cat_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2);

#ifdef __cplusplus
}
#endif

#endif
