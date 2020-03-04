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
#include "rex/rexfragment.h"

/**
 * Set the destination state (dstuid) of all dangling
 * transitions to point to uid.
 */
static void rex_fragment_patch(rarray_t *dangling, size_t uid)
{
	size_t i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(dangling); i++) {
		t = r_array_index(dangling, i, rex_transition_t *);
		t->dstuid = uid;
	}
}


static void rex_fragment_append(rarray_t *dest, rarray_t *src)
{
	size_t i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(src); i++) {
		t = r_array_index(src, i, rex_transition_t *);
		r_array_add(dest, &t);
	}
}

static void rex_fragment_state_add_dangling(rexfragment_t *frag, rexstate_t *state)
{
	size_t i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		if (t->dstuid == ((size_t)-1))
			r_array_add(frag->dangling, &t);
	}
	for (i = 0; i < r_array_length(state->etrans); i++) {
		t = (rex_transition_t *)r_array_slot(state->etrans, i);
		if (t->dstuid == ((size_t)-1))
			r_array_add(frag->dangling, &t);
	}
}

/*
 * The state must have all transitions in place, otherwise they
 * will not be added to the dangling array.
 */
rexfragment_t *rex_fragment_create(rexstate_t *state)
{
	size_t i;
	rex_transition_t *t;
	rexfragment_t *frag;
	frag = (rexfragment_t*)r_malloc(sizeof(*frag));
	r_memset(frag, 0, sizeof(*frag));
	frag->dangling = r_array_create(sizeof(rex_transition_t*));
	frag->sstate = state;

	rex_fragment_state_add_dangling(frag, state);
	return frag;
}


void rex_fragment_destroy(rexfragment_t *frag)
{
	if (frag)
		r_array_destroy(frag->dangling);
	r_free(frag);
}

/**
 * Concatinate two fragments
 */
rexfragment_t *rex_fragment_cat(rexdb_t *rexdb, rexfragment_t *frag1, rexfragment_t *frag2)
{
	rex_fragment_patch(frag1->dangling, REX_FRAG_STATEUID(frag2));
	frag2->sstate = frag1->sstate;
	rex_fragment_destroy(frag1);

	/*
	 * We return a fragment (frag2) which consists of the
	 * frag1->sstate
	 * and
	 * frag2->dangling
	 */
	return frag2;
}

/**
 *   --->[frag]--->
 *  |
 * >O
 *  |
 *   ------------->
 * We create a start state ">O" (state2) for the new fragment.
 */
rexfragment_t *rex_fragment_opt(rexdb_t *rexdb, rexfragment_t *frag1)
{
	rexstate_t *state2 = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
	rex_state_addtransition_e(state2, -1);
	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag1));
	frag1->sstate = state2;
	rex_fragment_state_add_dangling(frag1, state2);
	return frag1;
}


/**
 *   --->[frag]---
 *  |             |
 * >O <-----------
 *  |
 *   -------------->
 * We create a start state ">O" (state2) for the new fragment.
 */
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

/**
 *     -------
 *    V       |
 * >[frag]--->O--->
 */
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

/**
 *   --->[frag1]--->
 *  |
 * >O
 *  |
 *   --->[frag2]--->
 */
rexfragment_t *rex_fragment_alt(rexdb_t *rexdb, rexfragment_t *frag1, rexfragment_t *frag2)
{
	size_t i;
	rex_transition_t *t;

	rexstate_t *state2 = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag1));
	rex_state_addtransition_e(state2, REX_FRAG_STATEUID(frag2));
	frag1->sstate = state2;
	rex_fragment_append(frag1->dangling, frag2->dangling);
	rex_fragment_destroy(frag2);
	return frag1;
}

