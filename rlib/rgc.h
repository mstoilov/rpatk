#ifndef _RASTGC_H_
#define _RASTGC_H_

#include "robject.h"
#include "rlist.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rgc_s {
	robject_t obj;
	rhead_t head[2];
	ruint active;
} rgc_t;


robject_t *r_gc_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
rgc_t *r_gc_create();

/*
 * Virtual methods implementation
 */
void r_gc_cleanup(robject_t *obj);
robject_t *r_gc_copy(const robject_t *obj);
void r_gc_deallocateall(rgc_t *gc);
void r_gc_add(rgc_t *gc, robject_t *obj);
rhead_t *r_gc_head(rgc_t *gc);


#ifdef __cplusplus
}
#endif

#endif
