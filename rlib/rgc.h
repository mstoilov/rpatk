/*
 *  Regular Pattern Analyzer (RPA)
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

#ifndef _RASTGC_H_
#define _RASTGC_H_

#include "rlib/robject.h"
#include "rlib/rlist.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rgc_s {
	robject_t obj;
	rhead_t head[2];
	ruinteger active;
} rgc_t;


robject_t *r_gc_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
rgc_t *r_gc_create();
void r_gc_destroy(rgc_t *gc);
void r_gc_deallocateall(rgc_t *gc);
void r_gc_add(rgc_t *gc, robject_t *obj);
rhead_t *r_gc_head(rgc_t *gc);

/*
 * Virtual methods implementation
 */
void r_gc_cleanup(robject_t *obj);
robject_t *r_gc_copy(const robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
