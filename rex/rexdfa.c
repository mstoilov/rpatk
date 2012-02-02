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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "rlib/rmem.h"
#include "rex/rexdfa.h"
#include "rex/rexdb.h"


struct rexdfa_ctx {
	unsigned long nstates;
	unsigned long ntrnas;
	unsigned long nsubstates;
	unsigned long naccsubstates;
};


rexdfa_t *rex_dfa_create(unsigned long nstates, unsigned long ntrans, unsigned long nsubstates, unsigned long naccsubstates)
{
	rexdfa_t *dfa = (rexdfa_t *)r_zmalloc(sizeof(*dfa));
	dfa->states = r_zmalloc(sizeof(rexdfs_t) * nstates);
	dfa->trans = r_zmalloc(sizeof(rexdft_t) * ntrans);
	dfa->substates = r_zmalloc(sizeof(rexdfss_t) * nsubstates);
	dfa->accsubstates = r_zmalloc(sizeof(rexdfss_t) * naccsubstates);
	dfa->nstates = nstates;
	dfa->ntrans = ntrans;
	dfa->nsubstates = nsubstates;
	dfa->naccsubstates = naccsubstates;
	return dfa;
}


void rex_dfa_destroy(rexdfa_t *dfa)
{
	if (dfa) {
		r_free(dfa->substates);
		r_free(dfa->accsubstates);
		r_free(dfa->states);
		r_free(dfa->trans);
		r_free(dfa);
	}
}


static void rex_dfa_fillstate(rexdb_t *db, rexdfa_t *dfa, struct rexdfa_ctx *ctx, rexstate_t *state)
{
	long i;
	rex_transition_t *t = NULL;
	rexdfs_t *s = &dfa->states[ctx->nstates++];
	s->type = state->type;
	s->trans = ctx->ntrnas;
	s->ntrans = r_array_length(state->trans);
	for (i = 0; i < s->ntrans; i++) {
		t = (rex_transition_t *)r_array_slot(state->trans, i);
		dfa->trans[s->trans + i].lowin = t->lowin;
		dfa->trans[s->trans + i].highin = t->highin;
		dfa->trans[s->trans + i].state = t->dstuid;
	}
	ctx->ntrnas += s->ntrans;
	s->substates = ctx->nsubstates;
	s->nsubstates = rex_subset_length(state->subset);
	for (i = 0; i < s->nsubstates; i++) {
		unsigned long uid = rex_subset_index(state->subset, i);
		rexsubstate_t *substate = rex_db_getsubstate(db, uid);
		dfa->substates[s->substates + i].uid = uid;
		dfa->substates[s->substates + i].type = substate->ss_type;
		dfa->substates[s->substates + i].userdata = substate->ss_userdata;
	}
	ctx->nsubstates += s->nsubstates;
	s->accsubstates = ctx->naccsubstates;
	s->naccsubstates = 0L;
	for (i = 0; i < s->nsubstates; i++) {
		unsigned long uid = rex_subset_index(state->subset, i);
		rexsubstate_t *substate = rex_db_getsubstate(db, uid);
		if (substate->ss_type == REX_STATETYPE_ACCEPT) {
			dfa->accsubstates[s->accsubstates + s->naccsubstates].uid = uid;
			dfa->accsubstates[s->accsubstates + s->naccsubstates].type = substate->ss_type;
			dfa->accsubstates[s->accsubstates + s->naccsubstates].userdata = substate->ss_userdata;
			s->naccsubstates++;
		}
	}
	ctx->naccsubstates += s->naccsubstates;
}


rexdfa_t *rex_dfa_create_from_db(rexdb_t *db)
{
	long i;
	rexdfa_t *dfa;
	struct rexdfa_ctx ctx;
	unsigned long nstates = rex_db_numstates(db);
	unsigned long ntrans = rex_db_numtransitions(db);
	unsigned long nsubstates = rex_db_numsubstates(db);
	unsigned long naccsubstates = rex_db_numaccsubstates(db);
	dfa = rex_dfa_create(nstates, ntrans, nsubstates, naccsubstates);
	r_memset(&ctx, 0, sizeof(ctx));

	for (i = 0; i < r_array_length(db->states); i++) {
		rexstate_t *state = rex_db_getstate(db, i);
		rex_dfa_fillstate(db, dfa, &ctx, state);
	}
	R_ASSERT(ctx.nstates == nstates);
	R_ASSERT(ctx.ntrnas == ntrans);
	R_ASSERT(ctx.nsubstates == nsubstates);
	R_ASSERT(ctx.naccsubstates == naccsubstates);
	return dfa;
}


rexdfs_t *rex_dfa_state(rexdfa_t *dfa, unsigned long nstate)
{
	rexdfs_t *s;
	if (nstate >= dfa->nstates)
		return NULL;
	s = &dfa->states[nstate];
	return s;
}


rexdft_t *rex_dfa_transition(rexdfa_t *dfa, unsigned long nstate, unsigned long ntrans)
{
	rexdft_t *t;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (ntrans >= s->ntrans)
		return NULL;
	t = &dfa->trans[s->trans + ntrans];
	return t;
}


rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, unsigned long nstate, unsigned long nsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (nsubstate >= s->nsubstates)
		return NULL;
	ss = &dfa->substates[s->substates + nsubstate];
	return ss;
}


rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, unsigned long nstate, unsigned long naccsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (naccsubstate >= s->naccsubstates)
		return NULL;
	ss = &dfa->accsubstates[s->accsubstates + naccsubstate];
	return ss;
}


long rex_dfa_next(rexdfa_t *dfa, unsigned long nstate, rexchar_t input)
{
	rexdft_t *t;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	long min, max, mid;

	if (!s || !s->ntrans)
		return 0L;
	min = 0;
	max = min + s->ntrans;
	while (max > min) {
		mid = (min + max)/2;
		t = rex_dfa_transition(dfa, nstate, mid);
		if (input >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	if (min > 0)
		--min;
	t = rex_dfa_transition(dfa, nstate, min);
	if (input >= t->lowin && input <= t->highin)
		return t->state;
	return 0;
}


void rex_dfa_dumpstate(rexdfa_t *dfa, unsigned long nstate)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdft_t *t = NULL;

	if (!s)
		return;
	fprintf(stdout, "State %ld", nstate);
	fprintf(stdout, " (");
	for (index = 0; index < s->nsubstates; index++) {
		fprintf(stdout, "%ld", dfa->substates[s->substates + index].uid);
		if (dfa->substates[s->substates + index].type == REX_STATETYPE_ACCEPT)
			fprintf(stdout, "*");
		if (index + 1 < s->nsubstates)
			fprintf(stdout, ",");
	}
	fprintf(stdout, ")");


	fprintf(stdout, ": ");
	if (s->type == REX_STATETYPE_ACCEPT) {
		fprintf(stdout, " REX_STATETYPE_ACCEPT ");
		fprintf(stdout, " (");
		for (index = 0; index < s->naccsubstates; index++) {
			fprintf(stdout, "%ld*", dfa->accsubstates[s->accsubstates + index].uid);
			if (index + 1 < s->naccsubstates)
				fprintf(stdout, ",");
		}
		fprintf(stdout, ")");
	} else if (s->type == REX_STATETYPE_DEAD) {
		fprintf(stdout, " REX_STATETYPE_DEAD ");
	} else if (s->type == REX_STATETYPE_START) {
		fprintf(stdout, " REX_STATETYPE_START ");
	}
	fprintf(stdout, "\n");
	for (index = 0; index < s->ntrans; index++) {
		t = &dfa->trans[s->trans + index];
		n = 0;
		if (t->lowin != t->highin) {
			if (isprint(t->lowin) && !isspace(t->lowin) && isprint(t->highin) && !isspace(t->highin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [%c - %c] ", t->lowin, t->highin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        [0x%X - 0x%X] ", t->lowin, t->highin);
		} else {
			if (isprint(t->lowin) && !isspace(t->lowin))
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        '%c' ", t->lowin);
			else
				n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "        0x%X ", t->lowin);
		}
		r_memset(buf + n, ' ', bufsize - n);
		n = 40;
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "-> %ld", t->state);
		fprintf(stdout, "%s\n", buf);
	}
	if (!s->ntrans)
		fprintf(stdout, "        (none)\n");
	fprintf(stdout, "\n");

}


