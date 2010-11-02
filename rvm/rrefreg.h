#ifndef _RREFREG_H_
#define _RREFREG_H_

#include "rvmcpu.h"
#include "rref.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rrefreg_s {
	rref_t ref;
	rvmreg_t reg;
} rrefreg_t;


rrefreg_t *r_refreg_create();
rrefreg_t *r_refreg_init(rrefreg_t *refreg);
rrefreg_t *r_refreg_copy(const rrefreg_t *refreg);
void r_refreg_destroy(rrefreg_t *refreg);


#ifdef __cplusplus
}
#endif

#endif
