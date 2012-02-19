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

#include "rlib/rgc.h"


robject_t *r_gc_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	rgc_t *gc = (rgc_t*)obj;

	r_object_init(obj, type, cleanup, copy);
	r_list_init(&gc->head[0]);
	return obj;
}


rgc_t *r_gc_create()
{
	rgc_t *gc = (rgc_t*)r_object_create(sizeof(*gc));
	r_gc_init((robject_t*)gc, R_OBJECT_GC, r_gc_cleanup, r_gc_copy);
	return gc;
}


void r_gc_destroy(rgc_t *gc)
{
	r_object_destroy((robject_t *)gc);
}


void r_gc_cleanup(robject_t *obj)
{
	rgc_t *gc = (rgc_t*)obj;
	r_gc_deallocateall(gc);
	r_object_cleanup(obj);
}


robject_t *r_gc_copy(const robject_t *obj)
{

	return NULL;
}


void r_gc_deallocateall(rgc_t *gc)
{
	robject_t *obj;
	rhead_t *head = r_gc_head(gc);

	while (!r_list_empty(head)) {
		obj = r_list_entry(r_list_first(head), robject_t, lnk);
		r_list_del(&obj->lnk);
		obj->gc = NULL;
		r_object_destroy(obj);
	}
}


void r_gc_add(rgc_t *gc, robject_t *obj)
{
	rhead_t *head = r_gc_head(gc);

	r_list_addt(head, &obj->lnk);
	r_object_gcset(obj, gc);
}


rhead_t *r_gc_head(rgc_t *gc)
{
	return &gc->head[gc->active];
}
