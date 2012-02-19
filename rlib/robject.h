/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#ifndef _ROBJECT_H_
#define _ROBJECT_H_

#include "rtypes.h"
#include "rlib/rlist.h"

#ifdef __cplusplus
extern "C" {
#endif


#define R_OBJECT_UNKNOWN 0
#define R_OBJECT_STRING 1
#define R_OBJECT_ARRAY 2
#define R_OBJECT_HARRAY 3
#define R_OBJECT_CARRAY 4
#define R_OBJECT_MAP 5
#define R_OBJECT_HASH 6
#define R_OBJECT_REF 7
#define R_OBJECT_JSOBJECT 8
#define R_OBJECT_GC 9
#define R_OBJECT_USER 256


typedef struct robject_s robject_t;
/*
 * This should be renamed to cleanup
 */
typedef void (*r_object_cleanupfun)(robject_t *ptr);
typedef robject_t* (*r_object_copyfun)(const robject_t *ptr);

struct robject_s {
	rlink_t lnk;
	rpointer gc;
	rpointer gprops;
	rpointer lprops;
	ruint32 type;
	ruint32 size;
	r_object_cleanupfun cleanup;
	r_object_copyfun copy;
};

robject_t *r_object_create(unsigned long size);
robject_t *r_object_copy(const robject_t *obj);
void r_object_destroy(robject_t *obj);
void r_object_cleanup(robject_t *obj);
void r_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
void r_object_typeset(robject_t *obj, ruint32 type);
ruint32 r_object_typeget(robject_t *obj);
void r_object_gcset(robject_t *obj, rpointer gc);
rpointer r_object_gcget(robject_t *obj);

/*
 * Virtual methods
 */
robject_t *r_object_v_copy(const robject_t *obj);
void r_object_v_cleanup(robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
