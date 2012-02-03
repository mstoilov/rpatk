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
#include "rlib/rstring.h"
#include "rex/rexdfa.h"


rexdfa_t *rex_dfa_create(rexuint_t nstates, rexuint_t ntrans, rexuint_t nsubstates, rexuint_t naccsubstates)
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


rexdfs_t *rex_dfa_state(rexdfa_t *dfa, rexuint_t nstate)
{
	rexdfs_t *s;
	if (nstate >= dfa->nstates)
		return NULL;
	s = REX_DFA_STATE(dfa, nstate);
	return s;
}


rexdft_t *rex_dfa_transition(rexdfa_t *dfa, rexuint_t nstate, rexuint_t ntrans)
{
	rexdft_t *t;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (ntrans >= s->ntrans)
		return NULL;
	t = REX_DFA_TRANSITION(dfa, nstate, ntrans);
	return t;
}


rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t nsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (nsubstate >= s->nsubstates)
		return NULL;
	ss = REX_DFA_SUBSTATE(dfa, nstate, nsubstate);
	return ss;
}


rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t naccsubstate)
{
	rexdfss_t *ss;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	if (!s)
		return NULL;
	if (naccsubstate >= s->naccsubstates)
		return NULL;
	ss = REX_DFA_ACCSUBSTATE(dfa, nstate, naccsubstate);
	return ss;
}


rexuint_t rex_dfa_next(rexdfa_t *dfa, rexuint_t nstate, rexchar_t input)
{
	rexdft_t *t;
	long mid, min = 0, max = min + REX_DFA_STATE(dfa, nstate)->ntrans;
	while (max > min) {
		mid = (min + max)/2;
		t = REX_DFA_TRANSITION(dfa, nstate, mid);
		if (input >= t->lowin) {
			min = mid + 1;
		} else {
			max = mid;
		}
	}
	min -= (min > 0) ? 1 : 0;
	t = REX_DFA_TRANSITION(dfa, nstate, min);
	return (input >= t->lowin && input <= t->highin) ? t->state : 0;
}


void rex_dfa_dumpstate(rexdfa_t *dfa, rexuint_t nstate)
{
	long index;
	char buf[240];
	int bufsize = sizeof(buf) - 1;
	int n = 0;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdft_t *t = NULL;

	if (!s)
		return;
	fprintf(stdout, "State %ld", (unsigned long)nstate);
	fprintf(stdout, " (");
	for (index = 0; index < s->nsubstates; index++) {
		fprintf(stdout, "%ld", (unsigned long)dfa->substates[s->substates + index].uid);
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
			fprintf(stdout, "%ld*", (unsigned long)dfa->accsubstates[s->accsubstates + index].uid);
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


