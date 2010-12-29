#ifndef _ROBJECT_H_
#define _ROBJECT_H_

#include "rtypes.h"
#include "rlist.h"

#ifdef __cplusplus
extern "C" {
#endif


#define R_OBJECT_UNKNOWN 0
#define R_OBJECT_STRING 1
#define R_OBJECT_ARRAY 2
#define R_OBJECT_HARRAY 3
#define R_OBJECT_CARRAY 4
#define R_OBJECT_HASH 5
#define R_OBJECT_REF 6
#define R_OBJECT_USER 256


typedef struct robject_s robject_t;
/*
 * This should be renamed to cleanup
 */
typedef void (*r_object_cleanupfun)(robject_t *ptr);
typedef robject_t* (*r_object_copyfun)(const robject_t *ptr);

struct robject_s {
	rlink_t lnk;
	rlist_t *lst;
	ruint32 type;
	ruint32 size;
	r_object_cleanupfun cleanup;
	r_object_copyfun copy;
};

robject_t *r_object_create(rsize_t size);
robject_t *r_object_copy(const robject_t *obj);
void r_object_destroy(robject_t *obj);
void r_object_cleanup(robject_t *obj);
void r_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
void r_object_typeset(robject_t *obj, ruint32 type);
ruint32 r_object_typeget(robject_t *obj);

/*
 * Virtual methods
 */
robject_t *r_object_v_copy(const robject_t *obj);
void r_object_v_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
