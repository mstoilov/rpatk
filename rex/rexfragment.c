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


static void rex_fragment_clear_states(rexfragment_t *frag)
{
	rexstate_t *start;
	rexstate_t *end;

	start = rex_db_getstate(frag->rexdb, frag->start);
	end = rex_db_getstate(frag->rexdb, frag->end);
	start->type = REX_STATETYPE_NONE;
	end->type = REX_STATETYPE_NONE;
}


static void rex_fragment_set_states(rexfragment_t *frag)
{
	rexstate_t *start;
	rexstate_t *end;

	start = rex_db_getstate(frag->rexdb, frag->start);
	end = rex_db_getstate(frag->rexdb, frag->end);
	start->type = REX_STATETYPE_START;
	end->type = REX_STATETYPE_ACCEPT;
}


void rex_fragment_init(rexfragment_t *frag, rexdb_t *rexdb)
{
	r_memset(frag, 0, sizeof(*frag));
	frag->rexdb = rexdb;
	frag->start = rex_db_createstate(rexdb, REX_STATETYPE_START);
	frag->end = rex_db_createstate(rexdb, REX_STATETYPE_ACCEPT);
}


void rex_fragment_singletransition(rexfragment_t *frag, unsigned int input)
{
	rex_db_addtrasition(frag->rexdb, input, frag->start, frag->end);
}


void rex_fragment_rangetransition(rexfragment_t *frag, unsigned int c1, unsigned int c2)
{
	rex_db_addrangetrasition(frag->rexdb, c1, c2, frag->start, frag->end);
}


rexfragment_t *rex_fragment_create(rexdb_t *rexdb)
{
	rexfragment_t *frag;
	frag = (rexfragment_t*)r_malloc(sizeof(*frag));
	rex_fragment_init(frag, rexdb);
	return frag;
}


rexfragment_t *rex_fragment_create_singletransition(rexdb_t *rexdb, unsigned int input)
{
	rexfragment_t *frag = rex_fragment_create(rexdb);
	rex_fragment_singletransition(frag, input);
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
	rex_fragment_clear_states(frag1);
	rex_fragment_clear_states(frag2);
	rex_db_addtrasition_e(rexdb, frag1->end, frag2->start);
	frag1->end = frag2->end;
	rex_fragment_destroy(frag2);
	rex_fragment_set_states(frag1);
	return frag1;
}


rexfragment_t *rex_fragment_opt(rexfragment_t *frag)
{
	rex_db_addtrasition_e(frag->rexdb, frag->start, frag->end);
	return frag;
}


rexfragment_t *rex_fragment_mop(rexfragment_t *frag)
{
	unsigned long start, end;
	rex_fragment_clear_states(frag);
	start = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	end = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	rex_db_addtrasition_e(frag->rexdb, start, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->end, end);
	rex_db_addtrasition_e(frag->rexdb, start, end);
	frag->start = start;
	frag->end = end;
	rex_fragment_set_states(frag);
	return frag;
}


rexfragment_t *rex_fragment_mul(rexfragment_t *frag)
{
	unsigned long start, end;
	rex_fragment_clear_states(frag);
	start = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	end = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, frag->end, frag->start);
	rex_db_addtrasition_e(frag->rexdb, start, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->end, end);
	frag->start = start;
	frag->end = end;
	rex_fragment_set_states(frag);
	return frag;
}


static void rexfragment_to_union(rexfragment_t *frag)
{
	unsigned long start, end;
	if (frag->type == FA_FRAGTYPE_UNION)
		return;
	rex_fragment_clear_states(frag);
	start = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	end = rex_db_createstate(frag->rexdb, REX_STATETYPE_NONE);
	rex_db_addtrasition_e(frag->rexdb, start, frag->start);
	rex_db_addtrasition_e(frag->rexdb, frag->end, end);
	frag->start = start;
	frag->end = end;
	frag->type = FA_FRAGTYPE_UNION;
	rex_fragment_set_states(frag);
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
	rex_fragment_clear_states(frag2);
	rex_db_addtrasition_e(rexdb, frag1->start, frag2->start);
	rex_db_addtrasition_e(rexdb, frag2->end, frag1->end);
	rex_fragment_destroy(frag2);
	return frag1;
}

