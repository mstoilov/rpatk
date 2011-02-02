#ifndef _RASTNODE_H_
#define _RASTNODE_H_

#include "robject.h"
#include "rharray.h"
#include "rcarray.h"

#ifdef __cplusplus
extern "C" {
#endif

#define R_OBJECT_ASTNODE (R_OBJECT_USER + 1)

typedef struct rastnode_s rastnode_t;


typedef enum {
	R_AST_NONE = 0,
	R_AST_STRING,
	R_AST_DOUBLE,
	R_AST_LONG,
} r_asttype_t;


typedef struct rastsource_s {
	rpointer ptr;
	rulong size;
} rastsource_t;

#define R_ASTVAL_WORD 0
#define R_ASTVAL_LONG 1
#define R_ASTVAL_DOUBLE 2
#define R_ASTVAL_POINTER 3
#define R_ASTVAL_STRING 4
#define R_ASTVAL_ARRAY 5

typedef struct rastval_s {
	union {
		rword w;
		rlong l;
		rdouble d;
		rpointer ptr;
		rstring_t *str;
		rcarray_t *arr;
		rastnode_t *node;
	} v;
	ruint type;
} rastval_t;


struct rastnode_s {
	robject_t obj;
	rastval_t val;
	rastsource_t src;
	rharray_t *props;
};


robject_t *r_astnode_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
rastnode_t *r_astnode_create();

/*
 * Virtual methods implementation
 */
void r_astnode_cleanup(robject_t *obj);
robject_t *r_astnode_copy(const robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
