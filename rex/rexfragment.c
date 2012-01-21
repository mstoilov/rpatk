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
	rexstate_t *start;

	start = rex_db_getstate(frag->rexdb, frag->start);
	start->type = statetype;
}


void rex_fragment_set_endstatetype(rexfragment_t *frag, rex_statetype_t statetype)
{
	rexstate_t *end;

	end = rex_db_getstate(frag->rexdb, frag->end);
	end->type = statetype;
}


rexstate_t *rex_fragment_startstate(rexfragment_t *frag)
{
	return rex_db_getstate(frag->rexdb, frag->start);
}


rexstate_t *rex_fragment_endstate(rexfragment_t *frag)
{
	return rex_db_getstate(frag->rexdb, frag->end);
}


void rex_fragment_init(rexfragment_t *frag, rexdb_t *rexdb, long start, long end)
{
	r_memset(frag, 0, sizeof(*frag));
	frag->rexdb = rexdb;
	frag->start = start;
	frag->end = end;
	if (start < 0)
		frag->start = rex_db_createstate(rexdb, REX_STATETYPE_NONE);
	if (end < 0)
		frag->end = rex_db_createstate(rexdb, REX_STATETYPE_NONE);
}


void rex_fragment_transition(rexfragment_t *frag, rexchar_t c1, rexchar_t c2)
{
	rex_db_addrangetrasition(frag->rexdb, c1, c2, frag->start, frag->end);
}


rexfragment_t *rex_fragment_createwithstates(rexdb_t *rexdb, long start, long end)
{
	rexfragment_t *frag;
	frag = (rexfragment_t*)r_malloc(sizeof(*frag));
	rex_fragment_init(frag, rexdb, start, end);
	return frag;
}


rexfragment_t *rex_fragment_create(rexdb_t *rexdb)
{
	return rex_fragment_createwithstates(rexdb, -1, -1);
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
	if (frag1->rexdb != frag2->rexdb || frag1->start < 0 || frag1->end < 0 || frag2->start < 0 || frag2->end < 0) {
		/*
		 * Error
		 */
		return NULL;
	}
	rex_db_addtrasition_e(rexdb, frag1->end, frag2->start);
	frag1->end = frag2->end;
	rex_fragment_destroy(frag2);
	return frag1;
}


rexfragment_t *rex_fragment_opt(rexfragment_t *frag)
{
	rex_db_addtrasition_e(frag->rexdb, frag->start, frag->end);
	return frag;
}

#define NEWFRAG

#ifdef NEWFRAG
rexfragment_t *rex_fragment_mop(rexfragment_t *frag)
{
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->start, frag->end);
	return frag;
}


rexfragment_t *rex_fragment_mul(rexfragment_t *frag)
{
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	return frag;
}

#endif

#ifdef OLDFRAG
rexfragment_t *rex_fragment_mop(rexfragment_t *frag)
{
	unsigned long start, end;
	start = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	end = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	rex_db_addtrasition_e(frag->rexdb, start, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->end, end);
	rex_db_addtrasition_e(frag->rexdb, start, end);
	frag->start = start;
	frag->end = end;
	return frag;
}


rexfragment_t *rex_fragment_mul(rexfragment_t *frag)
{
	unsigned long start, end;
	start = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	end = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	rex_db_addtrasition_e(frag->rexdb, start, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->end, end);
	frag->start = start;
	frag->end = end;
	return frag;
}
#endif

static void rexfragment_to_union(rexfragment_t *frag)
{
	unsigned long newend;
	rexstate_t *endstate = rex_db_getstate(frag->rexdb, frag->end);
	if (endstate->type == REX_STATETYPE_NONE)
		return;
	newend = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, frag->end, newend);
	frag->end = newend;
}


rexfragment_t *rex_fragment_alt(rexfragment_t *frag1, rexfragment_t *frag2)
{
	rexdb_t *rexdb = frag1->rexdb;
	if (frag1->rexdb != frag2->rexdb || frag1->start < 0 || frag1->end < 0 || frag2->start < 0 || frag2->end < 0) {
		/*
		 * Error
		 */
		return NULL;
	}
	rexfragment_to_union(frag1);
	rex_db_addtrasition_e(rexdb, frag1->start, frag2->start);
	rex_db_addtrasition_e(rexdb, frag2->end, frag1->end);
	rex_fragment_destroy(frag2);
	return frag1;
}

