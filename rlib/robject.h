#ifndef _ROBJECT_H_
#define _ROBJECT_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


#define R_OBJECT_UNKNOWN 0
#define R_OBJECT_STRING 1
#define R_OBJECT_ARRAY 2
#define R_OBJECT_HARRAY 3
#define R_OBJECT_HASH 3
#define R_OBJECT_REFREG 4
#define R_OBJECT_USER 256


typedef struct robject_s robject_t;
typedef void (*r_object_destroyfun)(robject_t *ptr);
typedef robject_t* (*r_object_copyfun)(const robject_t *ptr);

struct robject_s {
	ruint32 type;
	r_object_destroyfun destroy;
	r_object_copyfun copy;
};

robject_t *r_object_copy(const robject_t *obj);
void r_object_destroy(robject_t *obj);
void r_object_init(robject_t *obj, ruint32 type, r_object_destroyfun destroy, r_object_copyfun copy);
void r_object_typeset(robject_t *obj, ruint32 type);
ruint32 r_object_typeget(robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
