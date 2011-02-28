#ifndef _RPAVM_H_
#define _RPAVM_H_

#include "rvmcpu.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RPA_RECORD_NONE (0)
#define RPA_RECORD_START (1 << 0)
#define RPA_RECORD_END (1 << 1)
#define RPA_RECORD_MATCH (1 << 2)

#define RPA_MATCH_NONE 0
#define RPA_MATCH_MULTIPLE (1 << 0)
#define RPA_MATCH_OPTIONAL (1 << 1)
#define RPA_MATCH_MULTIOPT (RPA_MATCH_MULTIPLE | RPA_MATCH_OPTIONAL)
#define R_MNODE_NAN R4
#define R_MNODE_MUL R5
#define R_MNODE_OPT R6
#define R_MNODE_MOP R7
#define R_ARG R8
#define R_WHT FP
#define R_TOP TP
#define RPAVM_SWI_TABLEID 0

#define RPA_MATCHCHR_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 0))
#define RPA_MATCHCHR_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 1))
#define RPA_MATCHCHR_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 2))
#define RPA_MATCHCHR_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 3))

#define RPA_MATCHRNG_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 4))
#define RPA_MATCHRNG_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 5))
#define RPA_MATCHRNG_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 6))
#define RPA_MATCHRNG_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 7))

#define RPA_MATCHSPCHR_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 8))
#define RPA_MATCHSPCHR_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 9))
#define RPA_MATCHSPCHR_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 10))
#define RPA_MATCHSPCHR_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 11))

#define RPA_SHIFT			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 12))
#define RPA_EMITSTART		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 13))
#define RPA_EMITEND			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 14))
#define RPA_BXLWHT			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 15))


typedef struct rparecord_s {
	rlist_t head;
	rlink_t lnk;
	const char *rule;
	rword top;
	rword size;
	rword type;
} rparecord_t;


typedef struct rpainput_s {
	const rchar *input;
	ruint32 wc;
	rint32 rp;
	ruchar eof;
} rpainput_t;


typedef struct rpainmap_s {
	const rchar *input;
	rulong serial;
} rpainmap_t;


rvmcpu_t *rpavm_cpu_create(rulong stacksize);
void rpavm_cpu_destroy(rvmcpu_t * vm);


#ifdef __cplusplus
}
#endif

#endif
