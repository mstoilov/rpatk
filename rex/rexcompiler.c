#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"
#include "rex/rextransition.h"


#define FPUSH(__co__, __fragment__) do { r_array_push((__co__)->stack, __fragment__, rexfragment_t*); } while (0)
#define FPOP(__co__) r_array_pop((__co__)->stack, rexfragment_t*)
#define FLEN(__co__) r_array_length((__co__)->stack)

static int rex_compiler_catexpression(rexcompiler_t *co);
static int rex_compiler_altexpression(rexcompiler_t *co);

enum {
	REX_TOKEN_ERR = -1,
	REX_TOKEN_EOF = -2,
};

rexcompiler_t *rex_compiler_create(rexdb_t *db)
{
	rexcompiler_t *co;

	co = (rexcompiler_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
	co->db = db;
	co->stack = r_array_create(sizeof(rexfragment_t*));
	co->temptrans = r_array_create(sizeof(rex_transition_t));
	return co;
}


void rex_compiler_destroy(rexcompiler_t *co)
{
	if (co) {
		r_array_destroy(co->stack);
		r_array_destroy(co->temptrans);
		r_free(co);
	}

}


static int rex_compiler_isspace(int c)
{
	return isspace(c);
}


static int rex_compiler_getchar(rexcompiler_t *co)
{
	ruint32 wc = REX_TOKEN_EOF;
	int inc = r_utf8_mbtowc(&wc, (const unsigned char*)co->ptr, (const unsigned char*)co->end);

	if (inc <= 0)
		return REX_TOKEN_EOF;
	co->ptr += inc;
	return wc;
}


static int rex_compiler_escapedchar(int c)
{
	switch (c) {
	case 'r':
		return '\r';
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case '0':
		return '\0';
	case 'f':
		return '\f';
	case 'v':
		return '\v';
	case '\"':
		return '\"';
	case '\'':
		return '\'';
	default:
		return c;
	}
	return c;
}


static int rex_compiler_gettok(rexcompiler_t *co)
{
	int c = rex_compiler_getchar(co);

	if (c == REX_TOKEN_EOF) {
		co->tokenstr[0] = '\0';
		co->token = REX_TOKEN_EOF;
		return co->token;
	} else {
		co->tokenstr[0] = c;
		co->tokenstr[1] = '\0';
		co->token = c;
	}
	return co->token;
}


/*
 * Don't do anything if this is not an escaped token
 */
static void rex_compiler_adjustescapedtoken(rexcompiler_t *co)
{
	if (co->token == '\\') {
		rex_compiler_gettok(co);		/* eat '\\' */
		if (co->token == REX_TOKEN_EOF) {
			return;
		}
		co->token = rex_compiler_escapedchar(co->token);
	}
}


static int rex_compiler_getnstok(rexcompiler_t *co)
{
	int tok = ' ';
	while (rex_compiler_isspace(tok))
		tok = rex_compiler_gettok(co);
	return tok;
}

#define REX_CONV_BUFSIZE 128
int rex_compiler_numerictoken(rexcompiler_t *co)
{
	char convbuf[REX_CONV_BUFSIZE + 1];
	int i = 0;
	int base = 0;

	rex_compiler_gettok(co); /* Eat # */
	if (co->token == 'x' || co->token == 'X') {
		rex_compiler_gettok(co);
		base = 16;
	} else if (co->token == '0') {
		rex_compiler_gettok(co);
		convbuf[i++] = '0';
		if (co->token == 'x' || co->token == 'X') {
			rex_compiler_gettok(co);
			i = 0;
			base = 16;
		} else {
			base = 8;
		}
	} else if (!isdigit(co->token)) {
		return -1;
	}
	while ((base == 16 && isxdigit(co->token)) || (base == 8 && isdigit(co->token) && co->token != '8' && co->token != '9') || isdigit(co->token)) {
		if (i >= REX_CONV_BUFSIZE)
			return -1;
		convbuf[i++] = co->token;
		if (rex_compiler_gettok(co) == REX_TOKEN_EOF)
			break;
	}
	convbuf[i++] = 0;
	co->numtoken = strtoul(convbuf, NULL, base);
	return 0;
}


int rex_compiler_charclass(rexcompiler_t *co)
{
	int negative = 0;
	int low;
	int high;
	long i;
	rex_transition_t *t;
	rexstate_t *srcstate = rex_db_getstate(co->db, rex_db_createstate(co->db, REX_STATETYPE_NONE));
	rexfragment_t *frag = NULL;

	rex_compiler_gettok(co);
	if (co->token == '^') {
		negative = 1;
		rex_compiler_gettok(co);
	}

	while (1) {
		if (co->token == '[' || co->token == ']' || co->token == '-' ||  co->token == REX_TOKEN_EOF)
			return -1;
		if (co->token == '#') {
			if (rex_compiler_numerictoken(co) < 0)
				return -1;
			high = low = co->numtoken;
		} else {
			rex_compiler_adjustescapedtoken(co);
			high = low = co->token;
			rex_compiler_gettok(co);
		}
		if (co->token == '-') {
			rex_compiler_gettok(co);
			switch (co->token) {
			case '[':
			case ']':
			case '-':
			case REX_TOKEN_EOF:
			case REX_TOKEN_ERR:
				return -1;
				break;
			}
			if (co->token == '#') {
				if (rex_compiler_numerictoken(co) < 0)
					return -1;
				high = co->numtoken;
			} else {
				rex_compiler_adjustescapedtoken(co);
				high = co->token;
				rex_compiler_gettok(co);
			}
		}
		rex_state_addtransition(srcstate, low, high, -1);
		if (co->token == ']') {
			rex_compiler_getnstok(co); /* eat it */
			break;
		}
	}
	rex_transitions_normalize(srcstate->trans);
	if (negative) {
		r_array_setlength(co->temptrans, 0);
		rex_transitions_negative(co->temptrans, srcstate->trans, srcstate->uid, -1);
		r_array_setlength(srcstate->trans, 0);
		for (i = 0; i < r_array_length(co->temptrans); i++) {
			t = (rex_transition_t *)r_array_slot(co->temptrans, i);
			r_array_add(srcstate->trans, t);
		}
		rex_transitions_normalize(srcstate->trans);
	}
	frag = rex_fragment_create(srcstate);
	FPUSH(co, frag);

	return 0;
}


static int rex_compiler_factor(rexcompiler_t *co)
{
	rexstate_t *srcstate;
	rexfragment_t *frag;

	if (co->token == '[') {
		return rex_compiler_charclass(co);
	} else if (co->token == '(') {
		rex_compiler_getnstok(co);		/* eat '(' */
		if (co->token == ')')
			return -1;
		if (rex_compiler_altexpression(co) < 0)
			return -1;
		if (co->token != ')')
			return -1;
		rex_compiler_getnstok(co);		/* eat ')' */
		return 0;
	} else if (co->token == '.') {
//		rexfragment_t *frag;
//		frag = rex_fragment_create(rex_db_getstate(co->db, rex_db_createstate(co->db, REX_STATETYPE_NONE)));
//		rex_fragment_danglingtransition(frag, 0, '\n' - 1);
//		rex_fragment_danglingtransition(frag, '\n' + 1, REX_CHAR_MAX);
//		rex_transitions_normalize(REX_FRAG_STATE(frag)->trans);
//		FPUSH(co, frag);
		srcstate = rex_db_getstate(co->db, rex_db_createstate(co->db, REX_STATETYPE_NONE));
		rex_compiler_adjustescapedtoken(co);
		frag = rex_fragment_create(srcstate);
		rex_state_addtransition(srcstate, 0, '\n' - 1, -1);
		rex_state_addtransition(srcstate, '\n' + 1, REX_CHAR_MAX, -1);
		frag = rex_fragment_create(srcstate);
		FPUSH(co, frag);
		rex_compiler_getnstok(co);
		return 0;
	} else {
		srcstate = rex_db_getstate(co->db, rex_db_createstate(co->db, REX_STATETYPE_NONE));
		rex_compiler_adjustescapedtoken(co);
		frag = rex_fragment_create(srcstate);
		rex_state_addtransition(srcstate, co->token, co->token, -1);
		frag = rex_fragment_create(srcstate);
		FPUSH(co, frag);
		rex_compiler_getnstok(co);
		return 0;
	}

	return -1;
}


static int rex_compiler_qfactor(rexcompiler_t *co)
{
	rexfragment_t *frag;

	if (strchr("*?+])", co->token)) {
		/*
		 * Unexpected char.
		 */
		return -1;
	}
	if (rex_compiler_factor(co) < 0) {
		return -1;
	}
	switch (co->token) {
	case '*':
		rex_compiler_getnstok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mop(co->db, frag);
		FPUSH(co, frag);
		break;
	case '?':
		rex_compiler_getnstok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_opt(co->db, frag);
		FPUSH(co, frag);
		break;
	case '+':
		rex_compiler_getnstok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mul(co->db, frag);
		FPUSH(co, frag);
		break;
	}
	return 0;
}


static int rex_compiler_catexpression(rexcompiler_t *co)
{
	int nfactors = 0;
	rexfragment_t *frag1;
	rexfragment_t *frag2;

	while (1) {
		switch (co->token) {
		case REX_TOKEN_EOF:
		case '|':
		case ')':
			return 0;
		}
		if (rex_compiler_qfactor(co) < 0)
			return -1;
		++nfactors;
		if (nfactors >= 2) {
			frag2 = FPOP(co);
			frag1 = FPOP(co);
			frag1 = rex_fragment_cat(co->db, frag1, frag2);
			FPUSH(co, frag1);
		}
	}
	return 0;
}


static int rex_compiler_altexpression(rexcompiler_t *co)
{
	rexfragment_t *frag1;
	rexfragment_t *frag2;
	if (rex_compiler_catexpression(co) < 0)
		return -1;
	while (co->token == '|') {
		rex_compiler_getnstok(co); /* Eat '|' */
		switch (co->token) {
		case REX_TOKEN_EOF:
		case '|':
		case ')':
		case ']':
			return -1;
		}
		if (rex_compiler_catexpression(co) < 0)
			return -1;
		frag2 = FPOP(co);
		frag1 = FPOP(co);
		frag1 = rex_fragment_alt(co->db, frag1, frag2);
		FPUSH(co, frag1);
	}
	return 0;
}


long rex_compiler_expression(rexcompiler_t *co, const char *str, unsigned int size, void *accdata)
{
	long curlen = r_array_length(co->db->states);
	long i;
	long ret = -1L;
	rexfragment_t *frag1 = NULL, *frag2 = NULL;

	co->start = str;
	co->end = str + size;
	co->ptr = str;

	rex_compiler_getnstok(co);
	if (rex_compiler_altexpression(co) < 0 || co->token != REX_TOKEN_EOF)
		goto error;
	frag1 = FPOP(co);
	frag2 = rex_fragment_create(rex_db_getstate(co->db, rex_db_createstate(co->db, REX_STATETYPE_ACCEPT)));
	frag1 = rex_fragment_cat(co->db, frag1, frag2);
	FPUSH(co, frag1);
	rex_state_setuserdata(REX_FRAG_STATE(frag1), accdata);
	ret = REX_FRAG_STATEUID(frag1);
	REX_FRAG_STATE(frag1)->type = REX_STATETYPE_START;
	rex_fragment_destroy(frag1);
	return ret;
error:
	if (curlen < r_array_length(co->db->states)) {
		for (i = curlen; i < r_array_length(co->db->states); i++)
			rex_state_destroy(rex_db_getstate(co->db, i));
		r_array_setlength(co->db->states, curlen);
	}
	while (FLEN(co)) {
		rex_fragment_destroy(FPOP(co));
	}
	return -1;
}


long rex_compiler_expression_s(rexcompiler_t *co, const char *str, void *accdata)
{
	return rex_compiler_expression(co, str, r_strlen(str), accdata);
}


long rex_compiler_addexpression(rexcompiler_t *co, unsigned long prev, const char *str, unsigned int size, void *accdata)
{
	rexstate_t *sprev = NULL, *scur = NULL;
	long cur = rex_compiler_expression(co, str, size, accdata);
	if (cur < 0)
		return -1;
	sprev = rex_db_getstate(co->db, prev);
	scur = rex_db_getstate(co->db, cur);
	if (!sprev)
		return scur->uid;
	rex_state_addtransition_e_dst(sprev, scur);
	scur->type = REX_STATETYPE_NONE;
	sprev->type = REX_STATETYPE_START;
	return prev;
}


long rex_compiler_addexpression_s(rexcompiler_t *co, unsigned long prev, const char *str, void *accdata)
{
	return rex_compiler_addexpression(co, prev, str, r_strlen(str), accdata);
}
