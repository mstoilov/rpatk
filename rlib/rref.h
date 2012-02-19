/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2010 Martin Stoilov
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

#ifndef _RREF_H_
#define _RREF_H_


#include "rtypes.h"
#include "rlib/robject.h"
#include "rlib/rspinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RREF_TYPE_SHARED = 0,
	RREF_TYPE_COW,
} rref_type_t;

typedef struct rref_s rref_t;

struct rref_s {
	robject_t obj;
	ruint32 count;
	rref_type_t type;
	rspinlock_t lock;
};

rref_t *r_ref_create(rref_type_t type);
robject_t *r_ref_init(robject_t *obj, ruint32 objtype, r_object_cleanupfun cleanup, r_object_copyfun copy, ruint32 count, rref_type_t type);

ruint32 r_ref_inc(rref_t *ref);
ruint32 r_ref_dec(rref_t *ref);
ruint32 r_ref_get(rref_t *ref);
void r_ref_typeset(rref_t *ref, rref_type_t type);
rref_type_t r_ref_typeget(rref_t *ref);

/*
 * Virtual methods implementation
 */
void r_ref_cleanup(robject_t *obj);
robject_t *r_ref_copy(const robject_t *obj);

#ifdef __cplusplus
}
#endif

#endif
