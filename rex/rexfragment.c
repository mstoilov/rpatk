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
	return frag->estate;
}


void rex_fragment_init(rexfragment_t *frag, rexdb_t *rexdb)
{
	r_memset(frag, 0, sizeof(*frag));
	frag->rexdb = rexdb;
	frag->sstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
	frag->estate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
}


void rex_fragment_transition(rexfragment_t *frag, rexchar_t c1, rexchar_t c2)
{
	rex_db_addrangetrasition(frag->rexdb, c1, c2, REX_FRAG_STARTUID(frag), REX_FRAG_ENDUID(frag));
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
	r_free(frag);
}


rexfragment_t *rex_fragment_cat(rexfragment_t *frag1, rexfragment_t *frag2)
{
	rexdb_t *rexdb;

	if (frag1 == NULL && frag2 != NULL)
		return frag2;
	else if (frag2 == NULL && frag1 != NULL)
		return frag1;
	else if (frag1 == NULL && frag2 == NULL)
		return NULL; /* Error */

	rexdb = frag1->rexdb;
	if (frag1->rexdb != frag2->rexdb || frag1->sstate == NULL || frag1->estate == NULL || frag2->sstate == NULL || frag2->estate == NULL) {
		/*
		 * Error
		 */
		return NULL;
	}
	rex_db_addtrasition_e(rexdb, REX_FRAG_ENDUID(frag1), REX_FRAG_STARTUID(frag2));
	frag1->estate = frag2->estate;
	rex_fragment_destroy(frag2);
	return frag1;
}


rexfragment_t *rex_fragment_opt(rexfragment_t *frag)
{
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_STARTUID(frag), REX_FRAG_ENDUID(frag));
	return frag;
}


rexfragment_t *rex_fragment_mop(rexfragment_t *frag)
{
	rexstate_t *start, *end;
	start = rex_db_getstate(frag->rexdb, rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE));
	end = rex_db_getstate(frag->rexdb, rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE));
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_ENDUID(frag), REX_FRAG_STARTUID(frag));
	rex_db_addtrasition_e(frag->rexdb, start->uid, REX_FRAG_STARTUID(frag));
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_ENDUID(frag), end->uid);
	rex_db_addtrasition_e(frag->rexdb, start->uid, end->uid);
	frag->sstate = start;
	frag->estate = end;
	return frag;
}


rexfragment_t *rex_fragment_mul(rexfragment_t *frag)
{
	rexstate_t *start, *end;
	start = rex_db_getstate(frag->rexdb, rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE));
	end = rex_db_getstate(frag->rexdb, rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE));
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_ENDUID(frag), REX_FRAG_STARTUID(frag));
	rex_db_addtrasition_e(frag->rexdb, start->uid, REX_FRAG_STARTUID(frag));
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_ENDUID(frag), end->uid);
	frag->sstate = start;
	frag->estate = end;
	return frag;
}


static void rexfragment_to_union(rexfragment_t *frag)
{
	unsigned long newend;
	rexstate_t *endstate = rex_db_getstate(frag->rexdb, REX_FRAG_ENDUID(frag));
	if (endstate->type == REX_STATETYPE_NONE)
		return;
	newend = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, REX_FRAG_ENDUID(frag), newend);
	frag->estate = rex_db_getstate(frag->rexdb, newend);
}


rexfragment_t *rex_fragment_alt(rexfragment_t *frag1, rexfragment_t *frag2)
{
	rexdb_t *rexdb = frag1->rexdb;
	if (frag1->rexdb != frag2->rexdb || frag1->sstate == NULL || frag1->estate == NULL || frag2->sstate == NULL || frag2->estate == NULL) {
		/*
		 * Error
		 */
		return NULL;
	}
	rexfragment_to_union(frag1);
	rex_db_addtrasition_e(rexdb, REX_FRAG_STARTUID(frag1), REX_FRAG_STARTUID(frag2));
	rex_db_addtrasition_e(rexdb, REX_FRAG_ENDUID(frag2), REX_FRAG_ENDUID(frag1));
	rex_fragment_destroy(frag2);
	return frag1;
}

