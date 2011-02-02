#ifndef _RASTALLOCATOR_H_
#define _RASTALLOCATOR_H_

#include "robject.h"
#include "rlist.h"
#include "rstring.h"
#include "rcarray.h"
#include "rastnode.h"


#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_ASTALLOCATOR (R_OBJECT_USER + 2)

typedef struct r_astgc_s {
	robject_t obj;
	rhead_t head;
} r_astgc_t;


robject_t *r_astgc_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
r_astgc_t *r_astgc_create();

/*
 * Virtual methods implementation
 */
void r_astgc_cleanup(robject_t *obj);
robject_t *r_astgc_copy(const robject_t *obj);
void r_astgc_deallocateall(r_astgc_t *gc);


#ifdef __cplusplus
}
#endif

#endif
