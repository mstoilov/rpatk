#ifndef _RVMGC_H_
#define _RVMGC_H_

#include "rtypes.h"
#include "rlist.h"
#include "robject.h"
#include "rvmreg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rvm_gc_s {
	rhead_t head[2];
	ruint active;

} rvm_gc_t;

void rvm_gc_destroy(rvm_gc_t *gc);
rvm_gc_t *rvm_gc_create();
rvm_gc_t *rvm_gc_init(rvm_gc_t *gc);
void rvm_gc_cleanup(rvm_gc_t *gc);

void rvm_gc_savelifes(rvm_gc_t *gc, rvmreg_t *array, ruint size);
int rvm_gc_add(rvm_gc_t *gc, robject_t *obj);
rhead_t *rvm_gc_activelist(rvm_gc_t *gc);
rhead_t *rvm_gc_inactivelist(rvm_gc_t *gc);
void rvm_gc_deallocate(rvm_gc_t *gc);
void rvm_gc_deallocate_all(rvm_gc_t *gc);


#ifdef __cplusplus
}
#endif

#endif
