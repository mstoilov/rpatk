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

#include "rlib/rmem.h"
#include "rexdfaconv.h"
#include "rextransition.h"


void rex_dfaconv_dumpsubset(rarray_t *subset)
{
	long index;
	if (rex_subset_length(subset)) {
		fprintf(stdout, " (");
		for (index = 0; index < rex_subset_length(subset); index++) {
			fprintf(stdout, "%ld", rex_subset_index(subset, index));
			fprintf(stdout, ",");
		}
		fprintf(stdout, ")");
	}
}


static unsigned int rex_dfaconv_subsethash(rconstpointer key)
{
	rarray_t *subset = (rarray_t *)key;
	unsigned long hash = 0;
	long i;

	for (i = 0; i < rex_subset_length(subset); i++)
		hash = rex_subset_index(subset, i) + (hash << 6) + (hash << 16) - hash;
	return hash;
}


static rboolean rex_dfaconv_subsetequal(rconstpointer key1, rconstpointer key2)
{
	long j;
	rarray_t *subset1 = (rarray_t *)key1;
	rarray_t *subset2 = (rarray_t *)key2;

	if (r_array_length(subset1) == r_array_length(subset2)) {
		for (j = 0; j < r_array_length(subset1); j++) {
			if (rex_subset_index(subset1, j) != rex_subset_index(subset2, j))
				break;
		}
		if (j == r_array_length(subset1)) {
			return TRUE;
		}
	}

	return FALSE;
}


rexdfaconv_t *rex_dfaconv_create()
{
	rexdfaconv_t *co;

	co = (rexdfaconv_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->setT = r_array_create(sizeof(unsigned long));
	co->setU = r_array_create(sizeof(unsigned long));
	co->setV = r_array_create(sizeof(unsigned long));
	co->trans = r_array_create(sizeof(rex_transition_t));
	co->temptrans = r_array_create(sizeof(rex_transition_t));
	co->marked = r_array_create(sizeof(unsigned char));
	co->temptrans1 = r_array_create(sizeof(rex_transition_t));
	co->ranges = r_array_create(sizeof(rexchar_t));
	co->hash = r_hash_create(16, rex_dfaconv_subsetequal, rex_dfaconv_subsethash);
	return co;
}


void rex_dfaconv_destroy(rexdfaconv_t *co)
{
	if (co) {
		r_array_destroy(co->setT);
		r_array_destroy(co->setU);
		r_array_destroy(co->setV);
		r_array_destroy(co->trans);
		r_array_destroy(co->temptrans);
		r_array_destroy(co->marked);
		r_hash_destroy(co->hash);
		r_array_destroy(co->temptrans1);
		r_array_destroy(co->ranges);
	}
	r_free(co);
}


static long rex_dfaconv_setsubstates(rexstate_t *state, rexdb_t *nfa, rarray_t *set)
{
	long i;
	unsigned long u;
	rexstate_t *s;

	rex_subset_clear(state->subset);
	for (i = 0; i < r_array_length(set); i++) {
		u = rex_subset_index(set, i);
		s = rex_db_getstate(nfa, u);
		rex_state_addsubstate(state, s);
		if (s->type == REX_STATETYPE_ACCEPT)
			state->type = REX_STATETYPE_ACCEPT;
	}
	return rex_subset_length(state->subset);
}


static void rex_dfaconv_mark(rexdfaconv_t *co, unsigned long uid)
{
	unsigned char marked = 1;
	r_array_replace(co->marked, uid, &marked);
}

static void rex_dfaconv_unmark(rexdfaconv_t *co, unsigned long uid)
{
	unsigned char marked = 0;
	r_array_replace(co->marked, uid, &marked);
}


static rboolean rex_dfaconv_ismarked(rexdfaconv_t *co, unsigned long uid)
{
	rboolean marked;
	if (uid >= r_array_length(co->marked))
		r_array_setlength(co->marked, uid + 1);
	marked = (rboolean)r_array_index(co->marked, uid, unsigned char);
//	fprintf(stdout, "%s, %ld : %d\n", __FUNCTION__, uid, marked);
	return marked;
}


static void rex_dfaconv_eclosure(rexdfaconv_t *co, rexdb_t *nfa, const rarray_t *src, rarray_t *dest)
{
	long i, j;
	unsigned long uid;
	rexstate_t *s;
	rex_transition_t *t;

	/*
	 * Empty the dest and temp stack(setT).
	 */
	rex_subset_clear(dest);
	rex_subset_clear(co->setT);
	for (i = 0; i < r_array_length(src); i++) {
		uid = rex_subset_index(src, i);
		rex_subset_addnewsubstate(co->setT, uid);
		rex_subset_addnewsubstate(dest, uid);
		rex_dfaconv_mark(co, uid);
	}
	while (!r_array_empty(co->setT)) {
		uid = rex_subset_pop(co->setT);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->etrans); j++) {
			t = (rex_transition_t *)r_array_slot(s->etrans, j);
			if (!rex_dfaconv_ismarked(co, t->dstuid)) {
				rex_subset_addnewsubstate(co->setT, t->dstuid);
				rex_subset_addnewsubstate(dest, t->dstuid);
				rex_dfaconv_mark(co, t->dstuid);
			}
		}
	}
	for (i = 0; i < r_array_length(dest); i++) {
		uid = rex_subset_index(dest, i);
		rex_dfaconv_unmark(co, uid);
	}
}


static void rex_dfaconv_getsubsettransitions(rexdfaconv_t *co, rexdb_t *nfa, rarray_t *states, rarray_t *trans)
{
	long i, j;
	long uid;
	rexstate_t *s;
	rex_transition_t *t;
	rex_transition_t temp;

	r_memset(&temp, 0, sizeof(temp));
	r_array_setlength(trans, 0);
	for (i = 0; i < r_array_length(states); i++) {
		uid = rex_subset_index(states, i);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			temp.lowin = t->lowin;
			temp.highin = t->highin;
			r_array_add(trans, &temp);
		}
	}
}


static void rex_dfaconv_move(rexdfaconv_t *co, rexdb_t *nfa, const rarray_t *substates, unsigned long c1, unsigned long c2, rarray_t *moveset)
{
	long i, j;
	long uid;
	rexstate_t *s;
	rex_transition_t *t;

	rex_subset_clear(moveset);
	for (i = 0; i < r_array_length(substates); i++) {
		uid = rex_subset_index(substates, i);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->trans); j++) {
			t = (rex_transition_t *)r_array_slot(s->trans, j);
			if ((c1 >= t->lowin && c2 <= t->highin)) {
				rex_subset_addnewsubstate(moveset, t->dstuid);
			}
		}
	}
}


static void rex_dfaconv_initsubstates(rexdb_t *dfa, rexdb_t *nfa)
{
	long i;
	rexsubstate_t substate;
	rexstate_t *s;
	rarray_t *substates = dfa->substates;

	r_array_setlength(substates, 0);
	for (i = 0; i < r_array_length(nfa->states); i++) {
		s = r_array_index(nfa->states, i, rexstate_t*);
		substate.ss_type = s->type;
		substate.ss_userdata = s->userdata;
		r_array_add(substates, &substate);
	}
}


void rex_dfaconv_uniquerange_insertvalue(rarray_t *ranges, rexchar_t value)
{
	long min, max, mid;
	rexchar_t existingval;

	min = 0;
	max = min + r_array_length(ranges);
	while (max > min) {
		mid = (min + max)/2;
		existingval = r_array_index(ranges, mid, rexchar_t);
		if (value == existingval) {
			return;
		} else if (value >= existingval) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	r_array_insert(ranges, min, &value);
}


void rex_dfaconv_uniqueranges(rexdfaconv_t *co, rarray_t *dest, rarray_t *src)
{
	long i;
	rex_transition_t *t;
	rexchar_t zero = 0, low, high;

	r_array_setlength(co->ranges, 0);
	r_array_add(co->ranges, &zero);
	for (i = 0; i < r_array_length(src); i++) {
		t = (rex_transition_t *)r_array_slot(src, i);
		rex_dfaconv_uniquerange_insertvalue(co->ranges, t->lowin);
		rex_dfaconv_uniquerange_insertvalue(co->ranges, t->highin + 1);
	}
	r_array_setlength(dest, 0);
	for (i = 0; i < r_array_length(co->ranges); i++) {
		if (i == r_array_length(co->ranges) - 1) {
			low = r_array_index(co->ranges, i, rexchar_t);
			high = REX_CHAR_MAX;
		} else {
			low = r_array_index(co->ranges, i, rexchar_t);
			high = (rexchar_t)r_array_index(co->ranges, i+1, rexchar_t) - 1;
		}
		rex_transitions_add(dest, low, high, 0, 0);
	}
}


rexdb_t *rex_dfaconv_run(rexdfaconv_t *co, rexdb_t *nfa, unsigned long start)
{
	long i, j;
	rexdb_t *dfa = rex_db_create(REXDB_TYPE_DFA);
	rexstate_t *s, *nextstate;
	rex_transition_t *t;
	long uid = rex_db_createstate(dfa, REX_STATETYPE_START);
	long *uidptr;

	rex_dfaconv_initsubstates(dfa, nfa);
	s = rex_db_getstate(dfa, uid);
	rex_subset_clear(co->setU);
	rex_subset_push(co->setU, start);
	rex_dfaconv_eclosure(co, nfa, co->setU, s->subset);
	r_hash_insert(co->hash, s->subset, &s->uid);

	for (i = 0; i < r_array_length(dfa->states); i++) {
		s = r_array_index(dfa->states, i, rexstate_t*);
		rex_dfaconv_getsubsettransitions(co, nfa, s->subset, co->temptrans);
		rex_dfaconv_uniqueranges(co, co->trans, co->temptrans);
		for (j = 0; j < r_array_length(co->trans); j++) {
			t = (rex_transition_t *)r_array_slot(co->trans, j);
			rex_dfaconv_move(co, nfa, s->subset, t->lowin, t->highin, co->setU);
			if (!rex_subset_length(co->setU)) {
				rex_state_addtransition(s, t->lowin, t->highin, 0);
				continue;
			}
			rex_dfaconv_eclosure(co, nfa, co->setU, co->setV);
			uidptr = r_hash_lookup(co->hash, co->setV);
			uid = uidptr ? *uidptr : -1;
			if (uid < 0) {
				uid = rex_db_createstate(dfa, REX_STATETYPE_NONE);
				nextstate = rex_db_getstate(dfa, uid);
				rex_dfaconv_setsubstates(nextstate, nfa, co->setV);
				r_hash_insert(co->hash, nextstate->subset, &nextstate->uid);
			}
			rex_state_addtransition(s, t->lowin, t->highin, uid);
		}
		rex_transitions_normalize(s->trans);
	}
	return dfa;
}
