#include "rmem.h"
#include "rvmgc.h"


rvm_gc_t *rvm_gc_create()
{
	rvm_gc_t *gc;

	gc = (rvm_gc_t *)r_zmalloc(sizeof(*gc));
	rvm_gc_init(gc);
	return gc;
}


rvm_gc_t *rvm_gc_init(rvm_gc_t *gc)
{
	r_list_init(&gc->head[0]);
	r_list_init(&gc->head[1]);
	gc->active = 0;
	return gc;
}


void rvm_gc_cleanup(rvm_gc_t *gc)
{
	r_memset(gc, 0, sizeof(gc));
}

void rvm_gc_destroy(rvm_gc_t *gc)
{
	rvm_gc_cleanup(gc);
	r_free(gc);
}

rhead_t *rvm_gc_activelist(rvm_gc_t *gc)
{
	return &gc->head[gc->active];
}


rhead_t *rvm_gc_inactivelist(rvm_gc_t *gc)
{
	return &gc->head[(gc->active + 1) & 1];
}


void rvm_gc_savelifes(rvm_gc_t *gc, rvmreg_t *array, ruint size)
{

}




void rvm_gc_deallocate(rvm_gc_t *gc)
{
	rlink_t *pos;
	rlist_t *inactiv = rvm_gc_inactivelist(gc);
	robject_t *obj;

	/*
	 * First pass: remove GC managed data from arrays
	 */
	r_list_foreach(pos, inactiv) {
		obj = r_list_entry(pos, robject_t, lnk);
		rvm_reg_array_unref_gcdata(obj);
	}

	while ((pos = r_list_first(inactiv)) != NULL) {
		obj = r_list_entry(pos, robject_t, lnk);
		r_list_del(pos);
		r_list_init(pos);
		r_object_destroy(obj);
	}
	gc->active = (gc->active + 1) & 1;
}


void rvm_gc_deallocate_all(rvm_gc_t *gc)
{
	gc->active = (gc->active + 1) & 1;
	rvm_gc_deallocate(gc);
	gc->active = (gc->active + 1) & 1;
}


int rvm_gc_add(rvm_gc_t *gc, robject_t *obj)
{
	rlist_t *activ = rvm_gc_activelist(gc);

	if (!r_list_empty(&obj->lnk))
		return 0;
	r_list_addt(activ, &obj->lnk);
#if 0
	if (obj->type == R_OBJECT_ARRAY) {
		/*
		 * Go through the data and add to gc. If the array is gc managed
		 * all the data in the array MUST be gc managed too.
		 */
		int i;
		rarray_t *array = (rarray_t*)obj;
		for (i = 0; i < r_array_length(array); i++) {
			rvmreg_t *r = (rvmreg_t *)r_array_slot(array, i);
			if (rvm_reg_tstflag(r, RVM_INFOBIT_ROBJECT)) {
				rvm_gc_add(gc, RVM_REG_GETP(r));
			}
		}
	} else if (obj->type == R_OBJECT_HARRAY) {
		/*
		 * Go through the data and add to gc. If the array is gc managed
		 * all the data in the array MUST be gc managed too.
		 */
		int i;
		rarray_t *array = ((rharray_t*)obj)->members;
		for (i = 0; i < r_array_length(array); i++) {
			rvmreg_t *r = (rvmreg_t *)r_array_slot(array, i);
			if (rvm_reg_tstflag(r, RVM_INFOBIT_ROBJECT)) {
				rvm_gc_add(gc, RVM_REG_GETP(r));
			}
		}

	} else {
		/*
		 * Nothing to do
		 */
	}
#endif
	return 0;
}
