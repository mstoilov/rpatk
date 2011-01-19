#ifndef __RVMOPERATORADD_H_
#define __RVMOPERATORADD_H_

#include "rvmoperator.h"
#include "rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

void rvm_op_add_unsigned(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rword op1, rword op2);
void rvm_op_add_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rlong op1, rlong op2);
void rvm_op_add_double(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, rdouble op1, rdouble op2);
void rvm_op_concat_string_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_concat_long_string(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_op_concat_string_long(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);


#ifdef __cplusplus
}
#endif


#endif
