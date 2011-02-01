#ifndef _RASTALLOCATOR_H_
#define _RASTALLOCATOR_H_

#include "robject.h"
#include "rlist.h"

#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_ASTALLOCATOR (R_OBJECT_USER + 2)

typedef struct r_astallocator_s {
	robject_t obj;
	rhead_t head;
} r_astallocator_t;


robject_t *r_astallocator_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
r_astallocator_t *r_astallocator_create();

/*
 * Virtual methods implementation
 */
void r_astallocator_cleanup(robject_t *obj);
robject_t *r_astallocator_copy(const robject_t *obj);
void r_astallocator_deallocateall(r_astallocator_t *allocator);


#ifdef __cplusplus
}
#endif

#endif
