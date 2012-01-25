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
#include "rex/rexfragment.h"

static void rex_fragment_patch(rarray_t *dangling, unsigned long uid)
{
	long i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(dangling); i++) {
		t = r_array_index(dangling, i, rex_transition_t *);
		t->dstuid = uid;
	}
}


static void rex_fragment_append(rarray_t *dest, rarray_t *src)
{
	long i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(src); i++) {
		t = r_array_index(src, i, rex_transition_t *);
		r_array_add(dest, &t);
	}
}

/*
 * The state must have all transitions in place, otherwise they
 * will not be added to the dangling array.
 */
rexfragment_t *rex_fragment_create(rexstate_t *state)
{
	long i;
	rex_transition_t *t;
	rexfragment_t *frag;
	frag = (rexfragment_t*)r_malloc(sizeof(*frag));
	r_memset(frag, 0, sizeof(*frag));
	frag->dangling = r_array_create(sizeof(rex_transition_t*));
	frag->sstate = state;

	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		if (t->dstuid < 0)
			r_array_add(frag->dangling, &t);
	}
	for (i = 0; i < r_array_length(state->etrans); i++) {
		t = (rex_transition_t *)r_array_slot(state->etrans, i);
		if (t->dstuid < 0)
			r_array_add(frag->dangling, &t);
	}
	return frag;
}


void rex_fragment_destroy(rexfragment_t *frag)
{
	if (frag)
		r_array_destroy(frag->dangling);
	r_free(frag);
}


rexfragment_t *rex_fragment_cat(rexdb_t *rexdb, rexfragment_t *frag1, rexfragment_t *frag2)
{
	rex_fragment_patch(frag1->dangling, REX_FRAG_STATEUID(frag2));
	frag2->sstate = frag1->sstate;
	rex_fragment_destroy(frag1);
	return frag2;
}


rexfragment_t *rex_fragment_opt(rexdb_t *rexdb, rexfragment_t *frag1)
{
	rexfragment_t *frag2;
	rexstate_t *state2 = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
	rex_state_addtransition_e(state2, -1);
	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag1));
	frag2 = rex_fragment_create(state2);
	rex_fragment_append(frag2->dangling, frag1->dangling);
	rex_fragment_destroy(frag1);
	return frag2;
}


rexfragment_t *rex_fragment_mop(rexdb_t *rexdb, rexfragment_t *frag1)
{
	rexfragment_t *frag2;
	rexstate_t *state2 = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));

	rex_state_addtransition_e(state2, -1);
	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag1));
	frag2 = rex_fragment_create(state2);
	rex_fragment_patch(frag1->dangling, REX_FRAG_STATEUID(frag2));
	rex_fragment_destroy(frag1);
	return frag2;
}


rexfragment_t *rex_fragment_mul(rexdb_t *rexdb, rexfragment_t *frag1)
{
	rexfragment_t *frag2;
	rexstate_t *state2 = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));

	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag1));
	rex_state_addtransition_e(state2, -1);
	frag2 = rex_fragment_create(state2);
	frag1 = rex_fragment_cat(rexdb, frag1, frag2);
	return frag1;
}


rexfragment_t *rex_fragment_alt(rexdb_t *rexdb, rexfragment_t *frag1, rexfragment_t *frag2)
{
	long i;
	rex_transition_t *t;

	rex_db_addtrasition_e(rexdb, REX_FRAG_STATEUID(frag1), REX_FRAG_STATEUID(frag2));
	for (i = 0; i < r_array_length(frag2->dangling); i++) {
		t = r_array_index(frag2->dangling, i, rex_transition_t *);
		r_array_add(frag1->dangling, &t);
	}
	rex_fragment_destroy(frag2);
	return frag1;
}

