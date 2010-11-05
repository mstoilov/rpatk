#ifndef _RREFREG_H_
#define _RREFREG_H_

#include "rvmcpu.h"
#include "rvmreg.h"
#include "rref.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rrefreg_s {
	rref_t ref;
	rvmreg_t reg;
} rrefreg_t;

#define REFREG2REG(__p__) ((rrefreg_t*)(__p__))->reg
#define REFREG2REGPTR(__p__) &REFREG2REG(__p__)
rrefreg_t *r_refreg_create();
void r_refreg_destroy(rrefreg_t *refreg);
rrefreg_t *r_refreg_init(rrefreg_t *refreg);
rrefreg_t *r_refreg_copy(const rrefreg_t* src);


#ifdef __cplusplus
}
#endif

#endif
