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

#include "rlib/rmem.h"
#include "rlib/robject.h"

/*
 * Allocate the memory for a new object
 */
robject_t *r_object_create(size_t size)
{
	robject_t *object;

	if ((object = (robject_t*)r_zmalloc(size)) == NULL)
		return NULL;
	object->size = size;
	object->gc = NULL;
	object->gprops = NULL;
	object->lprops = NULL;
	return object;
}

/*
 * Initialize a new object, setting up the cleanup and copy function pointers.
 * Initialize the linked list node.
 */
void r_object_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	r_list_init(&obj->lnk);
	obj->type = type;
	obj->cleanup = cleanup;
	obj->copy = copy;
}

robject_t *r_object_copy(const robject_t *obj)
{
	robject_t *newobj = r_object_create(obj->size);
	if (newobj)
		r_object_init(newobj, obj->type, obj->cleanup, obj->copy);
	return newobj;
}

/*
 * The cleanup doesn't allocate dynamic memory,
 * but if the object is on a linked list we will
 * remove it.
 *
 * This cleanup function must be called before
 * the destroy function, to make sure the object is
 * removed from a list.
 *
 * If this is a subclass inheriting from robject_t, then
 * the r_object_cleanup must be called last as part
 * of the cleanup calls chain.
 */
void r_object_cleanup(robject_t *obj)
{
	/*
	 * if on a list, remove it
	 */
	if (!r_list_empty(&obj->lnk))
		r_list_del(&obj->lnk);
	r_memset(obj, 0, sizeof(*obj));
}

/*
 * Cleanup the object and deallocate the memory
 */
void r_object_destroy(robject_t *obj)
{
	if (obj) {
		r_object_cleanupfun cleanup = obj->cleanup;
		/*
		 * Invoke the cleanup function. If this is a subclass
		 * it must call the r_object_cleanup as part of the
		 * cleanup chain.
		 */
		if (cleanup)
			cleanup(obj);

		/*
		 * Free the memory.
		 */
		r_free(obj);
	}
}

/*
 * Set the object type
 */
void r_object_typeset(robject_t *obj, ruint32 type)
{
	obj->type = type;
}

/*
 * Get the object type
 */
ruint32 r_object_typeget(robject_t *obj)
{
	return obj->type;
}

/*
 * Set the object garbage collection pointer
 */
void r_object_gcset(robject_t *obj, rpointer gc)
{
	obj->gc = gc;
}

/*
 * Get the object garbage collection pointer
 */
rpointer r_object_gcget(robject_t *obj)
{
	return obj->gc;
}
