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

#include "rlib/rmem.h"
#include "rlib/robject.h"


robject_t *r_object_create(unsigned long size)
{
	robject_t *object;

	if ((object = (robject_t*)r_zmalloc(size)) == NULL)
		return NULL;
	return object;
}

void r_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	r_list_init(&obj->lnk);
	obj->gc = NULL;
	obj->type = type;
	obj->cleanup = cleanup;
	obj->copy = copy;
}


robject_t *r_object_copy(const robject_t *obj)
{
	/*
	 * Nothing to do
	 */
	return NULL;
}


void r_object_cleanup(robject_t *obj)
{
	/*
	 * if on a list, remove it
	 */
	if (!r_list_empty(&obj->lnk))
		r_list_del(&obj->lnk);
	/*
	 * Nothing to do here, but for now lets wipe out the structure
	 */
	r_memset(obj, 0, sizeof(*obj));
}


void r_object_destroy(robject_t *obj)
{
	if (obj) {
		r_object_v_cleanup(obj);
		r_free(obj);
	}
}


robject_t *r_object_v_copy(const robject_t *obj)
{
	if (obj->copy)
		return obj->copy(obj);
	return NULL;
}


void r_object_v_cleanup(robject_t *obj)
{
	r_object_cleanupfun cleanup = obj->cleanup;
	if (cleanup)
		cleanup(obj);
}



void r_object_typeset(robject_t *obj, ruint32 type)
{
	obj->type = type;
}


ruint32 r_object_typeget(robject_t *obj)
{
	return obj->type;
}


void r_object_gcset(robject_t *obj, rpointer gc)
{
	obj->gc = gc;
}


rpointer r_object_gcget(robject_t *obj)
{
	return obj->gc;
}
