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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rexcc.h"


static rexdfa_t *tokens_dfa = NULL;
struct tokeninfo_s {
	int id;
	const char *name;
	const char *regex;
};


struct parseinfo_s {
	rbuffer_t id;
	rbuffer_t regex;
	int line;
};


#define REXCC_TOKEN_NONE			0
#define REXCC_TOKEN_DELIMITER		1
#define REXCC_TOKEN_IDENTIFIER		2
#define REXCC_TOKEN_SPACE			3
#define REXCC_TOKEN_CR				4
#define REXCC_TOKEN_REGEX			5


static struct tokeninfo_s tokens[] = {
		{REXCC_TOKEN_DELIMITER,		"delimiter",	"%%[ \\t]?[\\r]?[\\n]"},
		{REXCC_TOKEN_IDENTIFIER,	"identifier",	"([^\\t\\r\\n\'\" ]+|\"([^\"\\n]|\\\\\")*\")+|'.+'"},
		{REXCC_TOKEN_SPACE,			"space",		"[ \\t]+"},
		{REXCC_TOKEN_CR,			"cr ",			"[\\r]?[\\n]"},
		{REXCC_TOKEN_REGEX,			"regex",		"[ \\t](\\\\[\\r]?[\\n]|.)+\\r?\\n"},
		{0, NULL, NULL},
};



rexdfa_t * rex_cc_tokensdfa()
{
	if (!tokens_dfa) {
		long start = -1;
		struct tokeninfo_s *ti = tokens;
		rexdb_t *nfadb, *dfadb;
		nfadb = rex_db_create(REXDB_TYPE_NFA);
		while (ti->regex) {
			start = rex_db_addexpression_s(nfadb, start, ti->regex, (rexuserdata_t)ti);
			if (start < 0) {
				rex_db_destroy(nfadb);
				return NULL;
			}
			++ti;
		}
		dfadb = rex_db_createdfa(nfadb, start);
		if (dfadb) {
			tokens_dfa = rex_db_todfa(dfadb, 0);
		}
		rex_db_destroy(dfadb);
		rex_db_destroy(nfadb);
	}
	return tokens_dfa;
}


rexcc_t *rex_cc_create()
{
	rexcc_t *pCC;
	
	pCC = (rexcc_t *)r_malloc(sizeof(*pCC));
	if (!pCC)
		return (void *)0;
	r_memset(pCC, 0, sizeof(*pCC));
	pCC->parseinfo = r_array_create(sizeof(struct parseinfo_s));
	pCC->nfa = rex_db_create(REXDB_TYPE_NFA);
	pCC->startuid = -1L;
	return pCC;
}


void rex_cc_destroy(rexcc_t *pCC)
{
	if (!pCC)
		return;
	r_array_destroy(pCC->parseinfo);
	rex_db_destroy(pCC->nfa);
	rex_dfa_destroy(pCC->dfa);
	r_free(pCC->temp);
	r_free(pCC);
	if (tokens_dfa) {
		rex_dfa_destroy(tokens_dfa);
		tokens_dfa = NULL;
	}
}


int rex_cc_load_pattern(rexcc_t *pCC, rbuffer_t *buf, rexuserdata_t userdata)
{
	pCC->startuid = rex_db_addexpression(pCC->nfa, pCC->startuid, buf->s, buf->size, userdata);
	if (pCC->startuid < 0) {
		return -1;
	}
	return 0;
}


int rex_cc_vfprintf(FILE *out, int indent, const char *format, va_list ap)
{
	while (indent > 0) {
		fprintf(out, "\t");
		--indent;
	}
	return vfprintf(out, format, ap);
}


int rex_cc_fprintf(FILE *out, int indent, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = rex_cc_vfprintf(out, indent, format, ap);
	va_end(ap);
	return ret;
}


static void rex_cc_output_gpl(FILE *out)
{
	static char *gpl =
			"/*\n"
			" *  Regular Pattern Analyzer Toolkit(RPA/Tk)\n"
			" *  Copyright (c) 2009-2012 Martin Stoilov\n"
			" *\n"
			" *  This program is free software: you can redistribute it and/or modify\n"
			" *  it under the terms of the GNU General Public License as published by\n"
			" *  the Free Software Foundation, either version 3 of the License, or\n"
			" *  (at your option) any later version.\n"
			" *\n"
			" *  This program is distributed in the hope that it will be useful,\n"
			" *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
			" *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
			" *  GNU General Public License for more details.\n"
			" *\n"
			" *  You should have received a copy of the GNU General Public License\n"
			" *  along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
			" *\n"
			" *  Martin Stoilov <martin@rpasearch.com>\n"
			" */\n";
	rex_cc_fprintf(out, 0, "%s\n", gpl);
}


static int rex_cc_output_statesubstates(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdfss_t *ss;
	struct parseinfo_s *pi;

	for (i = 0; i < s->nsubstates; i++) {
		ss = rex_dfa_substate(dfa, nstate, i);
		if (ss->type == REX_STATETYPE_ACCEPT) {
			pi = (struct parseinfo_s *)r_array_slot(pCC->parseinfo, ss->userdata);
			pCC->temp = r_realloc(pCC->temp, pi->id.size + 1);
			r_memset(pCC->temp, 0, pi->id.size + 1);
			r_memcpy(pCC->temp, pi->id.s, pi->id.size);
			rex_cc_fprintf(out, 1, "{ %16lu, %16lu, (rexuserdata_t)(%s) },\n", (unsigned long)ss->type, (unsigned long)ss->uid, pCC->temp);
		} else {
			rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", (unsigned long)ss->type, (unsigned long)ss->uid, (unsigned long)0);
		}
	}
	return 0;
}


static int rex_cc_output_substates(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfss_t substates[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_statesubstates(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0UL, 0UL, 0UL);
	rex_cc_fprintf(out, 0, "};\n");
	return 0;
}


static int rex_cc_output_stateaccsubstates(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdfss_t *ss;
	struct parseinfo_s *pi;

	for (i = 0; i < s->naccsubstates; i++) {
		ss = rex_dfa_accsubstate(dfa, nstate, i);
		pi = (struct parseinfo_s *)r_array_slot(pCC->parseinfo, ss->userdata);
		pCC->temp = r_realloc(pCC->temp, pi->id.size + 1);
		r_memset(pCC->temp, 0, pi->id.size + 1);
		r_memcpy(pCC->temp, pi->id.s, pi->id.size);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, (rexuserdata_t)(%s) },\n", (unsigned long)ss->type, (unsigned long)ss->uid, pCC->temp);
	}
	return 0;
}


static int rex_cc_output_accsubstates(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfss_t accsubstates[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_stateaccsubstates(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0UL, 0UL, 0UL);
	rex_cc_fprintf(out, 0, "};\n");
	return 0;
}


static int rex_cc_output_statetransitions(rexcc_t *pCC, FILE *out, long nstate)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;
	rexdfs_t *s = rex_dfa_state(dfa, nstate);
	rexdft_t *t;
	for (i = 0; i < s->ntrans; i++) {
		t = rex_dfa_transition(dfa, nstate, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", (unsigned long)t->lowin, (unsigned long)t->highin, (unsigned long)t->state);
	}
	return 0;
}


static int rex_cc_output_transitions(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdft_t transitions[] = {\n");
	for (i = 0; i < dfa->nstates; i++)
		rex_cc_output_statetransitions(pCC, out, i);
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu },\n", 0UL, 0UL, 0UL);
	rex_cc_fprintf(out, 0, "};\n");
	return 0;
}


static int rex_cc_output_states(rexcc_t *pCC, FILE *out)
{
	long i;
	rexdfs_t *s;
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfs_t states[] = {\n");
	for (i = 0; i < dfa->nstates; i++) {
		s = rex_dfa_state(dfa, i);
		rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu, %16lu, %16lu, %16lu , %16lu},\n",
				(unsigned long)s->type, (unsigned long)s->trans, (unsigned long)s->ntrans, (unsigned long)s->accsubstates,
				(unsigned long)s->naccsubstates, (unsigned long)s->substates, (unsigned long)s->nsubstates);

	}
	rex_cc_fprintf(out, 1, "{ %16lu, %16lu, %16lu, %16lu, %16lu, %16lu , %16lu},\n", 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL);
	rex_cc_fprintf(out, 0, "};\n");
	return 0;
}


static int rex_cc_output_dfa(rexcc_t *pCC, FILE *out)
{
	rexdfa_t *dfa = pCC->dfa;

	rex_cc_fprintf(out, 0, "static rexdfa_t ccdfa = {\n");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->nstates);
	rex_cc_fprintf(out, 1, "%s,\n", "states");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->ntrans);
	rex_cc_fprintf(out, 1, "%s,\n", "transitions");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->naccsubstates);
	rex_cc_fprintf(out, 1, "%s,\n", "accsubstates");
	rex_cc_fprintf(out, 1, "%lu,\n", dfa->nsubstates);
	rex_cc_fprintf(out, 1, "%s,\n", "substates");
	rex_cc_fprintf(out, 1, "{0, },\n");
	rex_cc_fprintf(out, 0, "};\n");
	return 0;
}



int rex_cc_output(rexcc_t *pCC, FILE *outc)
{
	if (outc) {
		rex_cc_output_gpl(outc);
		rex_cc_fprintf(outc, 0, "#include \"rexdfa.h\"\n\n");
		if (pCC->prolog.size) {
			fwrite(pCC->prolog.s, 1, pCC->prolog.size, outc);
			rex_cc_fprintf(outc, 0, "\n");
		}
		rex_cc_output_accsubstates(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_substates(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_transitions(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_states(pCC, outc);
		rex_cc_fprintf(outc, 0, "\n\n");
		rex_cc_output_dfa(pCC, outc);
		if (pCC->epilog.size) {
			rex_cc_fprintf(outc, 0, "\n");
			fwrite(pCC->epilog.s, 1, pCC->epilog.size, outc);
		}
	}
	return 0;
}


static int rex_cc_getlineno(rexcc_t *pCC, const char *input)
{
	int ret = 1;

	while (--input >= pCC->start) {
		if (*input == '\n')
			ret += 1;
	}
	return ret;
}


int rex_cc_gettoken(rexcc_t *pCC)
{
	ruint32 wc = 0;
	int inc, ret = 0;
	long nstate = REX_DFA_STARTSTATE;
	const char *input = pCC->input;
	rexdfa_t *dfa = rex_cc_tokensdfa();
	rexdfs_t *s = NULL;
	rexdfss_t *ss = NULL;

	pCC->token = 0;
	pCC->tokenlen = 0;
	if (!dfa) {
		/*
		 * Error
		 */
		return -1;
	}
	while ((inc = r_utf8_mbtowc(&wc, (const unsigned char*)input, (const unsigned char*)pCC->end)) > 0) {
		REX_DFA_NEXT(dfa, nstate, wc, &nstate);
		if (nstate == 0)
			break;
		input += inc;
		s = REX_DFA_STATE(dfa, nstate);
		ss = REX_DFA_ACCSUBSTATE(dfa, nstate, 0);
		if (s->type == REX_STATETYPE_ACCEPT) {
			pCC->token = ((struct tokeninfo_s *)ss->userdata)->id;
			ret = input - pCC->input;
			if (ss && ((struct tokeninfo_s *)ss->userdata)->id == 1) {
				break;
			}
		}
	}
	if (ret) {
		pCC->tokenptr = pCC->input;
		pCC->tokenlen = ret;
		pCC->input += ret;
	}
	return ret;
}


static int rex_cc_parseid(rexcc_t *pCC, struct parseinfo_s *info)
{
	info->id.s = pCC->tokenptr;
	info->id.size = pCC->tokenlen;
	rex_cc_gettoken(pCC);
	return 0;
}


static int rex_cc_parseregex(rexcc_t *pCC, struct parseinfo_s *info)
{
	info->regex.s = pCC->tokenptr;
	info->regex.size = pCC->tokenlen;
	rex_cc_gettoken(pCC);
	return 0;
}


static int rex_cc_parseline(rexcc_t *pCC)
{
	struct parseinfo_s info;

	r_memset(&info, 0, sizeof(info));
	if (rex_cc_parseid(pCC, &info) < 0)
		return -1;
	if (pCC->token != REXCC_TOKEN_REGEX) {
		/*
		 * Unexpected char.
		 */
		fprintf(stdout, "Line %d, (%s) Unexpected Char.\n", rex_cc_getlineno(pCC, pCC->input), "Error");
		return -1;
	}
	if (rex_cc_parseregex(pCC, &info) < 0)
		return -1;
	r_array_add(pCC->parseinfo, &info);
	return 0;
}


int rex_cc_parse(rexcc_t *pCC)
{
	pCC->prolog.s = pCC->input;
	pCC->prolog.size = 0;
	while (pCC->input + 3 < pCC->end) {
		if (*pCC->input == '%' && *(pCC->input+1) == '%' && (*(pCC->input+2) == '\n' || (*(pCC->input+2) == '\r' && *(pCC->input+3) == '\n'))) {
			break;
		}
		pCC->prolog.size += 1;
		pCC->input += 1;
	}
	rex_cc_gettoken(pCC);
	if (pCC->token != REXCC_TOKEN_DELIMITER)
		return -1;
	rex_cc_gettoken(pCC);
	while (pCC->token) {
		if (pCC->token == REXCC_TOKEN_CR || pCC->token == REXCC_TOKEN_SPACE || pCC->token == REXCC_TOKEN_REGEX) {
			rex_cc_gettoken(pCC);
		} else if (pCC->token == REXCC_TOKEN_IDENTIFIER) {
			if (rex_cc_parseline(pCC) < 0)
				return -1;
		} else if (pCC->token == REXCC_TOKEN_DELIMITER) {
			rex_cc_gettoken(pCC);
			return 0;
		} else {
			/*
			 * Unexpected char
			 */
			fprintf(stdout, "Line %d, (%s) Unexpected Char.\n", rex_cc_getlineno(pCC, pCC->input), "Error");
			return -1;
		}
		pCC->epilog.s = pCC->input;
		pCC->epilog.size = pCC->end - pCC->input;
	}
	return -1;
}


int rex_cc_load_buffer(rexcc_t *pCC, rbuffer_t *text)
{
	int ret = 0, i;
	struct parseinfo_s *pi;

	pCC->start = text->s;
	pCC->input = text->s;
	pCC->end = text->s + text->size;
	r_array_setlength(pCC->parseinfo, 0);
	if (rex_cc_parse(pCC) == 0) {
		for (i = 0; i < r_array_length(pCC->parseinfo); i++) {
			pi = (struct parseinfo_s *)r_array_slot(pCC->parseinfo, i);
			if (rex_cc_load_pattern(pCC, &pi->regex, i) < 0) {
				fprintf(stdout, "Line %d, (%s) Syntax error: ", rex_cc_getlineno(pCC, pi->id.s), "Error");
				fwrite(pi->id.s, 1, pi->id.size, stdout);
				fprintf(stdout, "  ");
				fwrite(pi->regex.s, 1, pi->regex.size, stdout);
				fprintf(stdout, "\n");
				return -1;
			}
#if 0
			fwrite(pi->id.s, 1, pi->id.size, stdout);
			fprintf(stdout, " : ");
			fwrite(pi->regex.s, 1, pi->regex.size, stdout);
			fprintf(stdout, "\n");
#endif
		}
	}
	return ret;
}


void rex_cc_parseinfodump(rexcc_t *pCC)
{
	long i;
	struct parseinfo_s *pi;

	for (i = 0; i < r_array_length(pCC->parseinfo); i++) {
		pi = (struct parseinfo_s *)r_array_slot(pCC->parseinfo, i);
		fwrite(pi->id.s, 1, pi->id.size, stdout);
		fprintf(stdout, "  ");
		fwrite(pi->regex.s, 1, pi->regex.size, stdout);
		fprintf(stdout, "\n");
	}
}
