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
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rextransition.h"
#include "rex/rexstate.h"


/*
 * Add new substate, while keep the array of substates sorted.
 */
void rex_subset_addnewsubstate(rarray_t *subset, size_t uid)
{
	long min, max, mid;
	size_t ss_uid;

	min = 0;
	max = min + r_array_length(subset);
	while (max > min) {
		mid = (min + max)/2;
		ss_uid = rex_subset_index(subset, mid);
		if (uid == ss_uid) {
			return;
		} else if (uid >= ss_uid) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(subset, min, &uid);
}


void rex_state_cleanup(robject_t *obj)
{
	rexstate_t *state = (rexstate_t*) obj;

	r_array_destroy(state->trans);
	r_array_destroy(state->etrans);
	r_array_destroy(state->subset);
	r_object_cleanup(obj);
}

robject_t* rex_state_copy(const robject_t *obj)
{
	rexstate_t *src = (rexstate_t*)obj;
	rexstate_t *state = (rexstate_t*)r_object_create(sizeof(*state));
	r_object_init((robject_t*)state, obj->type, obj->cleanup, obj->copy);
	state->trans = (rarray_t*)src->trans->obj.copy((robject_t *)src->trans);
	state->etrans = (rarray_t*)src->etrans->obj.copy((robject_t *)src->etrans);
	state->subset = (rarray_t*)src->subset->obj.copy((robject_t *)src->subset);
	state->type = src->type;
	state->uid = src->uid;
	return (robject_t *)state;
}

void rex_state_init(robject_t *obj, ruint32 objtype, r_object_cleanupfun cleanup, r_object_copyfun copy, size_t uid, rex_statetype_t statetype)
{
	rexstate_t *state = (rexstate_t*)obj;

	r_object_init(obj, objtype, cleanup, copy);
	state->trans = r_array_create(sizeof(rex_transition_t));
	state->etrans = r_array_create(sizeof(rex_transition_t));
//	state->subset = r_array_create(sizeof(rexsubstate_t));
	state->subset = r_array_create(sizeof(size_t));
	state->type = statetype;
	state->uid = uid;
}

rexstate_t *rex_state_create(size_t uid, rex_statetype_t statetype)
{
	rexstate_t *state;
	state = (rexstate_t*)r_object_create(sizeof(*state));
	rex_state_init((robject_t*)state, R_OBJECT_FASTATE, rex_state_cleanup, rex_state_copy, uid, statetype);
	return state;
}


void rex_state_destroy(rexstate_t *state)
{
	r_object_destroy((robject_t*)state);
}

rex_transition_t * rex_state_addtransition_e(rexstate_t *state, size_t dstuid)
{
	return rex_transitions_add_e(state->etrans, state->uid, dstuid);
}


rex_transition_t * rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate)
{
	return rex_state_addtransition_e(srcstate, dststate->uid);
}


rex_transition_t *rex_state_addtransition(rexstate_t *state, rexchar_t c1, rexchar_t c2, size_t dstuid)
{
	return rex_transitions_add(state->trans, c1, c2, state->uid, dstuid);
}


rex_transition_t *rex_state_addtransition_dst(rexstate_t *srcstate, rexchar_t c1, rexchar_t c2, const rexstate_t *dststate)
{
	return rex_state_addtransition(srcstate, c1, c2, dststate->uid);
}


void rex_state_addsubstate(rexstate_t *state, size_t uid, size_t type, rexuserdata_t userdata)
{
	long min, max, mid;
	size_t ss_uid;
	rexsubstate_t newsubstate;
	newsubstate.ss_uid = uid;
	newsubstate.ss_type = type;
	newsubstate.ss_userdata = userdata;

	min = 0;
	max = min + r_array_length(state->subset);
	while (max > min) {
		mid = (min + max)/2;
		ss_uid = ((rexsubstate_t*)r_array_slot(state->subset, mid))->ss_uid;
		if (newsubstate.ss_uid == ss_uid) {
			return;
		} else if (newsubstate.ss_uid >= ss_uid) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(state->subset, min, &newsubstate);
}


void rex_state_addsubstate_dst(rexstate_t *state, const rexstate_t *substate)
{
	rex_state_addsubstate(state, substate->uid, substate->type, substate->userdata);
}


void rex_state_setuserdata(rexstate_t *state, rexuserdata_t userdata)
{
	state->userdata = userdata;
}


size_t rex_state_getuserdata(rexstate_t *state)
{
	return state->userdata;
}
