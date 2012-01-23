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


void rex_fragment_set_startstatetype(rexfragment_t *frag, rex_statetype_t statetype)
{
	rex_fragment_startstate(frag)->type = statetype;
}


void rex_fragment_set_endstatetype(rexfragment_t *frag, rex_statetype_t statetype)
{
	rex_fragment_endstate(frag)->type = statetype;
}


rexstate_t *rex_fragment_startstate(rexfragment_t *frag)
{
	return frag->sstate;
}


rexstate_t *rex_fragment_endstate(rexfragment_t *frag)
{
	return NULL;
}


void rex_fragment_init(rexfragment_t *frag, rexdb_t *rexdb)
{
	r_memset(frag, 0, sizeof(*frag));
	frag->dangling = r_array_create(sizeof(rex_transition_t*));
	frag->rexdb = rexdb;
	frag->sstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
}


void rex_fragment_transition(rexfragment_t *frag, rexchar_t c1, rexchar_t c2)
{
	rex_transition_t *t = rex_db_addrangetrasition(frag->rexdb, c1, c2, REX_FRAG_STATEUID(frag), 0);
	r_array_add(frag->dangling, &t);
}


void rex_fragment_transition_e(rexfragment_t *frag)
{
	rex_transition_t *t = rex_db_addtrasition_e(frag->rexdb, REX_FRAG_STATEUID(frag), 0);
	r_array_add(frag->dangling, &t);
}


rexfragment_t *rex_fragment_create(rexdb_t *rexdb)
{
	rexfragment_t *frag;
	frag = (rexfragment_t*)r_malloc(sizeof(*frag));
	rex_fragment_init(frag, rexdb);
	return frag;
}


rexfragment_t *rex_fragment_create_transition(rexdb_t *rexdb, rexchar_t c1, rexchar_t c2)
{
	rexfragment_t *frag = rex_fragment_create(rexdb);
	rex_fragment_transition(frag, c1, c2);
	return frag;
}


void rex_fragment_destroy(rexfragment_t *frag)
{
	if (frag)
		r_array_destroy(frag->dangling);
	r_free(frag);
}


rexfragment_t *rex_fragment_cat(rexfragment_t *frag1, rexfragment_t *frag2)
{
	long i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(frag1->dangling); i++) {
		t = r_array_index(frag1->dangling, i, rex_transition_t *);
		t->dstuid = REX_FRAG_STATEUID(frag2);
	}
	frag2->sstate = frag1->sstate;
	rex_fragment_destroy(frag1);
	return frag2;
}


rexfragment_t *rex_fragment_opt(rexfragment_t *frag)
{
	rexstate_t *split = REX_FRAG_STATE(frag);
	rex_transition_t *t = rex_state_addtransition_e(split, 0);
	r_array_add(frag->dangling, &t);
	return frag;
}


rexfragment_t *rex_fragment_mop(rexfragment_t *frag)
{
	long i;
	rex_transition_t *t;

	for (i = 0; i < r_array_length(frag->dangling); i++) {
		t = r_array_index(frag->dangling, i, rex_transition_t *);
		t->dstuid = REX_FRAG_STATEUID(frag);
	}
	r_array_setlength(frag->dangling, 0);
	t = rex_state_addtransition_e(REX_FRAG_STATE(frag), 0);
	r_array_add(frag->dangling, &t);
	return frag;
}


rexfragment_t *rex_fragment_mul(rexfragment_t *frag1)
{
	rexfragment_t *frag2 = rex_fragment_create(frag1->rexdb);
	rex_db_addtrasition_e(frag1->rexdb, REX_FRAG_STATEUID(frag2), REX_FRAG_STATEUID(frag1));
	rex_fragment_transition_e(frag2);
	frag1 = rex_fragment_cat(frag1, frag2);
	return frag1;
}


rexfragment_t *rex_fragment_alt(rexfragment_t *frag1, rexfragment_t *frag2)
{
	long i;
	rex_transition_t *t;

	rex_db_addtrasition_e(frag1->rexdb, REX_FRAG_STATEUID(frag1), REX_FRAG_STATEUID(frag2));
	for (i = 0; i < r_array_length(frag2->dangling); i++) {
		t = r_array_index(frag2->dangling, i, rex_transition_t *);
		r_array_add(frag1->dangling, &t);
	}
	rex_fragment_destroy(frag2);
	return frag1;
}

