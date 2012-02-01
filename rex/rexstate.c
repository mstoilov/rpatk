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
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rextransition.h"
#include "rex/rexstate.h"



void rex_subset_addnewsubstate(rarray_t *subset, unsigned long uid)
{
	long min, max, mid;
	unsigned long ss_uid;

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


robject_t *rex_state_init(robject_t *obj, unsigned int objtype, r_object_cleanupfun cleanup, unsigned long uid, unsigned long statetype)
{
	rexstate_t *state = (rexstate_t*)obj;

	r_object_init(obj, objtype, cleanup, NULL);
	state->trans = r_array_create(sizeof(rex_transition_t));
	state->etrans = r_array_create(sizeof(rex_transition_t));
	state->subset = r_array_create(sizeof(rexsubstate_t));
	state->type = statetype;
	state->uid = uid;

	return (robject_t*)state;
}


rexstate_t *rex_state_create(unsigned long uid, long statetype)
{
	rexstate_t *state;
	state = (rexstate_t*)r_object_create(sizeof(*state));
	rex_state_init((robject_t*)state, R_OBJECT_FASTATE, rex_state_cleanup, uid, statetype);
	return state;
}


void rex_state_destroy(rexstate_t *state)
{
	r_object_destroy((robject_t*)state);
}

rex_transition_t * rex_state_addtransition_e(rexstate_t *state, unsigned long dstuid)
{
	return rex_transitions_add_e(state->etrans, state->uid, dstuid);
}


rex_transition_t * rex_state_addtransition_e_dst(rexstate_t *srcstate, const rexstate_t *dststate)
{
	return rex_state_addtransition_e(srcstate, dststate->uid);
}


rex_transition_t *rex_state_addtransition(rexstate_t *state, rexchar_t c1, rexchar_t c2, unsigned long dstuid)
{
	return rex_transitions_add(state->trans, c1, c2, state->uid, dstuid);
}


rex_transition_t *rex_state_addtransition_dst(rexstate_t *srcstate, rexchar_t c1, rexchar_t c2, const rexstate_t *dststate)
{
	return rex_state_addtransition(srcstate, c1, c2, dststate->uid);
}


void rex_state_addnewsubstate(rexstate_t *state, unsigned long ss_uid)
{
	rex_subset_addnewsubstate(state->subset, ss_uid);
}


void rex_state_addsubstate(rexstate_t *state, const rexstate_t *substate)
{
	rex_state_addnewsubstate(state, substate->uid);
}


void rex_state_setuserdata(rexstate_t *state, rexuserdata_t userdata)
{
	state->userdata = userdata;
}


unsigned long rex_state_getuserdata(rexstate_t *state)
{
	return state->userdata;
}

long rex_state_next(rexstate_t *state, unsigned long input)
{
	rex_transition_t *t;
	long i = 0;

	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		if (t->lowin == input)
			return t->dstuid;
	}
	return -1;
}

