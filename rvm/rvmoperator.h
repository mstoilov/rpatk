#ifndef _RVMOPERATOR_H_
#define _RVMOPERATOR_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rvm/rvmcpu.h"

#define RVM_UNARY_HANDLER(__t__) (__t__)
#define RVM_OP_HANDLER(__ft__, __st__) ((__ft__) * RVM_DTYPE_MAX + (__st__))


enum {
	RVM_OPID_NONE = 0,
	RVM_OPID_ADD,
	RVM_OPID_SUB,
	RVM_OPID_MUL,
	RVM_OPID_DIV,
	RVM_OPID_CAT,
	RVM_OPID_CAST,
	RVM_OPID_LSL,
	RVM_OPID_LSR,
	RVM_OPID_LSRU,
	RVM_OPID_AND,
	RVM_OPID_XOR,
	RVM_OPID_OR,
	RVM_OPID_NOT,
	RVM_OPID_CMP,
	RVM_OPID_CMN,
	RVM_OPID_MOD,
	RVM_OPID_LOGICAND,
	RVM_OPID_LOGICOR,
	RVM_OPID_LOGICNOT,
	RVM_OPID_EQ,
	RVM_OPID_NOTEQ,
	RVM_OPID_LESS,
	RVM_OPID_LESSEQ,
	RVM_OPID_GREATER,
	RVM_OPID_GREATEREQ,

	RVM_OPID_USER0,
	RVM_OPID_USER1,
	RVM_OPID_USER2,
	RVM_OPID_USER3,
	RVM_OPID_USER4,
	RVM_OPID_USER5,
	RVM_OPID_USER6,
	RVM_OPID_USER7,
	RVM_OPID_USER8,
	RVM_OPID_USER9,
	RVM_OPID_USER10,
	RVM_OPID_USER11,
	RVM_OPID_USER12,
	RVM_OPID_USER13,
	RVM_OPID_USER14,
	RVM_OPID_USER15,
	RVM_OPID_LAST
};

/*
#define RVM_OPID_NONE 0
#define RVM_OPID_ADD 1
#define RVM_OPID_SUB 2
#define RVM_OPID_MUL 3
#define RVM_OPID_DIV 4
#define RVM_OPID_CAT 5
#define RVM_OPID_CAST 6
#define RVM_OPID_LSL 7
#define RVM_OPID_LSR 8
#define RVM_OPID_LSRU 8

#define RVM_OPID_AND 9
#define RVM_OPID_XOR 10
#define RVM_OPID_OR 11
#define RVM_OPID_NOT 12
#define RVM_OPID_CMP 13
#define RVM_OPID_CMN 14
#define RVM_OPID_MOD 15
#define RVM_OPID_LESS 16

#define RVM_OPID_LAST 32
*/
/*
 * Important: the res pointer might be the same as one of the arguments, the operator must
 * be implemented to take care of such cases.
 */
typedef void (*rvm_unaryop_handler)(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg);
typedef void (*rvm_binaryop_handler)(rvmcpu_t *cpu, rushort opid, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);

typedef union rvm_ophandler_s {
	rvm_unaryop_handler unary;
	rvm_binaryop_handler op;
} rvm_ophandler_t;

typedef struct rvm_opinfo_s {
	rushort opid;
	rboolean unary;
	rvm_ophandler_t *handlers;
} rvm_opinfo_t;

typedef struct rvm_opmap_s {
	rarray_t *operators;
} rvm_opmap_t;


rvm_opmap_t *rvm_opmap_create();
void rvm_opmap_destroy(rvm_opmap_t *opmap);
void rvm_opmap_add_binary_operator(rvm_opmap_t *opmap, rushort opid);
void rvm_opmap_add_unary_operator(rvm_opmap_t *opmap, rushort opid);
rinteger rvm_opmap_set_binary_handler(rvm_opmap_t *opmap, rushort opid, rvm_binaryop_handler func, ruchar firstType, ruchar secondType);
rinteger rvm_opmap_set_unary_handler(rvm_opmap_t *opmap, rushort opid, rvm_unaryop_handler func, ruchar type);
void rvm_opmap_invoke_binary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg1, const rvmreg_t *arg2);
void rvm_opmap_invoke_unary_handler(rvm_opmap_t *opmap, rushort opid, rvmcpu_t *cpu, rvmreg_t *res, const rvmreg_t *arg);

#endif
