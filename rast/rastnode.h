#ifndef _RASTNODE_H_
#define _RASTNODE_H_

#include "robject.h"
#include "rharray.h"
#include "rcarray.h"

#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_ASTNODE (R_OBJECT_USER + 1)

typedef struct r_astnode_s r_astnode_t;

typedef enum {
	R_AST_NONE = 0,
	R_AST_STRING,
	R_AST_DOUBLE,
	R_AST_LONG,
} r_asttype_t;


struct r_astnode_s {
	robject_t obj;
	r_asttype_t type;
	rcarray_t *childs;
	rharray_t *props;
};


robject_t *r_astnode_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
r_astnode_t *r_astnode_create();

/*
 * Virtual methods implementation
 */
void r_astnode_cleanup(robject_t *obj);
robject_t *r_astnode_copy(const robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
