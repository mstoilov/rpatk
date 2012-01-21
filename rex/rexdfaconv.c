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
	co->hash = r_hash_create(12, rex_dfaconv_subsetequal, rex_dfaconv_subsethash);
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
		r_hash_destroy(co->hash);
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


static rboolean rex_dfaconv_finduid(rarray_t *subset, unsigned long uid)
{
	long i;

	for (i = 0; i < r_array_length(subset); i++) {
		if (rex_subset_index(subset, i) == uid)
			return TRUE;
	}
	return FALSE;
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
	}
	for (i = 0; i < r_array_length(co->setT); i++) {
		uid = rex_subset_index(co->setT, i);
		s = rex_db_getstate(nfa, uid);
		for (j = 0; j < r_array_length(s->etrans); j++) {
			t = (rex_transition_t *)r_array_slot(s->etrans, j);
			if (rex_dfaconv_finduid(co->setT, t->dstuid))
				continue;
			rex_subset_addnewsubstate(co->setT, t->dstuid);
			rex_subset_addnewsubstate(dest, t->dstuid);
		}
	}



//	while (!r_array_empty(co->setT)) {
//		uid = rex_subset_pop(co->setT);
//		s = rex_db_getstate(nfa, uid);
//		for (j = 0; j < r_array_length(s->etrans); j++) {
//			t = (rex_transition_t *)r_array_slot(s->etrans, j);
//			rex_subset_addnewsubstate(co->setT, t->dstuid);
//			rex_subset_addnewsubstate(dest, t->dstuid);
//		}
//	}
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


static void rex_dfaconv_insertdeadtransitions(rexdfaconv_t *co, rexstate_t *state)
{
	long i;
	rex_transition_t *t;

	r_array_setlength(co->temptrans, 0);
	for (i = 0; i < r_array_length(state->trans); i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		rex_transitions_add(co->temptrans, t->lowin, t->highin, state->uid, 0);
	}
	rex_transitions_normalize(co->temptrans);
	rex_transitions_negative(state->trans, co->temptrans, state->uid, 0);
	rex_transitions_normalize(state->trans);
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
		rex_transitions_uniqueranges(co->trans, co->temptrans);
		for (j = 0; j < r_array_length(co->trans); j++) {
			t = (rex_transition_t *)r_array_slot(co->trans, j);
			rex_dfaconv_move(co, nfa, s->subset, t->lowin, t->highin, co->setU);
			if (!rex_subset_length(co->setU))
				continue;
			rex_dfaconv_eclosure(co, nfa, co->setU, co->setV);
//			if ((uid = rex_db_findstate(dfa, co->setV)) < 0) {
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
		rex_dfaconv_insertdeadtransitions(co, s);
	}
	return dfa;
}
