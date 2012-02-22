#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"
#include "rex/rextransition.h"
#include "rex/rexstate.h"
#include "rex/rexfragment.h"


struct rexcompiler_s {
	rarray_t *stack;
	rarray_t *temptrans;
	const char *start;
	const char *end;
	const char *ptr;
	const char *tokenptr;
	int token;
	rexchar_t numtoken;
	unsigned long fromcount;
	unsigned long tocount;
	char tokenstr[REX_COMPILER_TOKENSIZE];
};


#define FPUSH(__co__, __fragment__) do { r_array_push((__co__)->stack, __fragment__, rexfragment_t*); } while (0)
#define FPOP(__co__) r_array_pop((__co__)->stack, rexfragment_t*)
#define FLEN(__co__) r_array_length((__co__)->stack)

static int rex_compiler_catexpression(rexcompiler_t *co, rexdb_t *rexdb);
static int rex_compiler_altexpression(rexcompiler_t *co, rexdb_t *rexdb);

enum {
	REX_TOKEN_ERR = -1,
	REX_TOKEN_EOF = -2,
};

rexcompiler_t *rex_compiler_create()
{
	rexcompiler_t *co;

	co = (rexcompiler_t *)r_malloc(sizeof(*co));
	r_memset(co, 0, sizeof(*co));
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


static int rex_compiler_isblank(int c)
{
	return r_strchr(" \t", c) ? 1 : 0;
}


static int rex_compiler_getchar(rexcompiler_t *co)
{
	ruint32 wc = REX_TOKEN_EOF;
	int inc = 0;

	inc = r_utf8_mbtowc(&wc, (const unsigned char*)co->ptr, (const unsigned char*)co->end);
	if (inc <= 0)
		return REX_TOKEN_EOF;
	if (wc == '\r' || wc == '\n') {
		return REX_TOKEN_EOF;
	}
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
	int c;

	co->tokenptr = co->ptr;
	c = rex_compiler_getchar(co);
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


static int rex_compiler_getnbtok(rexcompiler_t *co)
{
again:
	while (co->ptr < co->end && rex_compiler_isblank(*co->ptr))
		co->ptr += 1;
	if (co->ptr + 1 < co->end && *co->ptr == '\\' && *(co->ptr + 1) == '\n') {
		co->ptr += 2;
		goto again;
	}
	if (co->ptr + 2 < co->end && *co->ptr == '\\' && *(co->ptr + 1) == '\r' && *(co->ptr + 2) == '\n') {
		co->ptr += 3;
		goto again;
	}

	return rex_compiler_gettok(co);

}


static int rex_compiler_getnstok(rexcompiler_t *co)
{
again:
	while (co->ptr < co->end && rex_compiler_isspace(*co->ptr))
		co->ptr += 1;
	if (co->ptr + 1 < co->end && *co->ptr == '\\' && *(co->ptr + 1) == '\n') {
		co->ptr += 2;
		goto again;
	}
	if (co->ptr + 2 < co->end && *co->ptr == '\\' && *(co->ptr + 1) == '\r' && *(co->ptr + 2) == '\n') {
		co->ptr += 3;
		goto again;
	}

	return rex_compiler_gettok(co);
}


#define REX_CONV_BUFSIZE 128
int rex_compiler_numerictoken(rexcompiler_t *co, rexdb_t *rexdb)
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


int rex_compiler_charclass(rexcompiler_t *co, rexdb_t *rexdb)
{
	int negative = 0;
	int low;
	int high;
	long i;
	rex_transition_t *t;
	rexstate_t *srcstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
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
			if (rex_compiler_numerictoken(co, rexdb) < 0)
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
				if (rex_compiler_numerictoken(co, rexdb) < 0)
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
			rex_compiler_getnbtok(co); /* eat it */
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


static int rex_compiler_factor(rexcompiler_t *co, rexdb_t *rexdb)
{
	rexstate_t *srcstate;
	rexfragment_t *frag;

	if (co->token == '[') {
		return rex_compiler_charclass(co, rexdb);
	} else if (co->token == '(') {
		rex_compiler_getnbtok(co);		/* eat '(' */
		if (co->token == ')')
			return -1;
		if (rex_compiler_altexpression(co, rexdb) < 0)
			return -1;
		if (co->token != ')')
			return -1;
		rex_compiler_getnbtok(co);		/* eat ')' */
		return 0;
	} else if (co->token == '.') {
		srcstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
		rex_compiler_adjustescapedtoken(co);
		rex_state_addtransition(srcstate, 0, '\n' - 1, -1);
		rex_state_addtransition(srcstate, '\n' + 1, REX_CHAR_MAX, -1);
		frag = rex_fragment_create(srcstate);
		FPUSH(co, frag);
		rex_compiler_getnbtok(co);
		return 0;
	} else {
		srcstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
		rex_compiler_adjustescapedtoken(co);
		rex_state_addtransition(srcstate, co->token, co->token, -1);
		frag = rex_fragment_create(srcstate);
		FPUSH(co, frag);
		rex_compiler_getnbtok(co);
		return 0;
	}

	return -1;
}


static int rex_compiler_parsecount(rexcompiler_t *co, rexdb_t *rexdb)
{
	char convbuf[REX_CONV_BUFSIZE + 1];
	int i = 0;

	rex_compiler_getnbtok(co);		/* eat '{' */
	if (!isdigit(co->token))
		return -1;
	for (i = 0; isdigit(co->token); i++) {
		if (i >= REX_CONV_BUFSIZE)
			return -1;
		convbuf[i] = co->token;
		if (rex_compiler_gettok(co) == REX_TOKEN_EOF)
			return -1;
	}
	convbuf[i++] = 0;
	co->fromcount = strtoul(convbuf, NULL, 10);
	co->tocount = co->fromcount;
	if (rex_compiler_isblank(co->token)) {
		rex_compiler_getnbtok(co);
	}
	if (co->token == ',') {
		rex_compiler_getnbtok(co);		/* eat ',' */
		co->tocount = (unsigned long)-1;
		if (isdigit(co->token)) {
			for (i = 0; isdigit(co->token); i++) {
				if (i >= REX_CONV_BUFSIZE)
					return -1;
				convbuf[i] = co->token;
				if (rex_compiler_gettok(co) == REX_TOKEN_EOF)
					return -1;
			}
			convbuf[i++] = 0;
			co->tocount = strtoul(convbuf, NULL, 10);
			if (rex_compiler_isblank(co->token)) {
				rex_compiler_getnbtok(co);
			}
		}
	}
	if (co->token != '}')
		return -1;
	rex_compiler_getnbtok(co);		/* eat '}' */
	return 0;
}


static int rex_compiler_qfactor(rexcompiler_t *co, rexdb_t *rexdb)
{
	rexfragment_t *frag, *frag1, *frag2;
	const char *fstart, *fend;
	rexstate_t *srcstate;
	long nstates, i;

	if (strchr("{}*?+]\n\r)", co->token)) {
		/*
		 * Unexpected char.
		 */
		return -1;
	}
	nstates = r_array_length(rexdb->states);
	fstart = co->tokenptr;
	if (rex_compiler_factor(co, rexdb) < 0) {
		return -1;
	}
	fend = co->tokenptr;
	switch (co->token) {
	case '*':
		rex_compiler_getnbtok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mop(rexdb, frag);
		FPUSH(co, frag);
		break;
	case '?':
		rex_compiler_getnbtok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_opt(rexdb, frag);
		FPUSH(co, frag);
		break;
	case '+':
		rex_compiler_getnbtok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mul(rexdb, frag);
		FPUSH(co, frag);
		break;
	case '{':
		if (rex_compiler_parsecount(co, rexdb) < 0)
			return -1;
		if (co->fromcount > co->tocount)
			return -1;
		if (co->fromcount == 0 && co->tocount == (unsigned long)-1) {
			frag = FPOP(co);
			frag = rex_fragment_mop(rexdb, frag);
			FPUSH(co, frag);
			break;
		}
		if (co->fromcount == 1 && co->tocount == (unsigned long)-1) {
			frag = FPOP(co);
			frag = rex_fragment_mul(rexdb, frag);
			FPUSH(co, frag);
			break;
		}
		if (co->fromcount == 0 && co->tocount == 0) {
			/*
			 * Special case
			 */
			frag = FPOP(co);
			for (i = nstates; i < r_array_length(rexdb->states); i++) {
				srcstate = rex_db_getstate(rexdb, i);
				rex_state_destroy(srcstate);
			}
			r_array_setlength(rexdb->states, nstates);
			srcstate = rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_NONE));
			rex_compiler_adjustescapedtoken(co);
			rex_state_addtransition_e(srcstate, -1);
			frag = rex_fragment_create(srcstate);
			FPUSH(co, frag);
			break;
		}
		if (co->fromcount == 0) {
			frag = FPOP(co);
			frag = rex_fragment_opt(rexdb, frag);
			FPUSH(co, frag);
		}
		do {
			long count;
			const char *saved_start = co->start;
			const char *saved_end = co->end;
			const char *saved_ptr = co->ptr;
			const char *saved_tokenptr = co->tokenptr;
			int saved_token = co->token;
			for (count = 1; count < co->tocount; count++) {
				co->start = fstart;
				co->end = fend;
				co->ptr = fstart;
				rex_compiler_getnbtok(co);
				rex_compiler_catexpression(co, rexdb);
				frag2 = FPOP(co);
				if (co->tocount == (unsigned long)-1 && count >= co->fromcount) {
					frag2 = rex_fragment_mop(rexdb, frag2);
					frag1 = FPOP(co);
					frag1 = rex_fragment_cat(rexdb, frag1, frag2);
					FPUSH(co, frag1);
					break;
				}
				if (count >= co->fromcount) {
					frag2 = rex_fragment_opt(rexdb, frag2);
				}
				frag1 = FPOP(co);
				frag1 = rex_fragment_cat(rexdb, frag1, frag2);
				FPUSH(co, frag1);
			}
			co->start = saved_start;
			co->end = saved_end;
			co->ptr = saved_ptr;
			co->tokenptr = saved_tokenptr;
			co->token = saved_token;
		} while (0);
		break;
	}
	return 0;
}


static int rex_compiler_catexpression(rexcompiler_t *co, rexdb_t *rexdb)
{
	int nfactors = 0;
	rexfragment_t *frag1;
	rexfragment_t *frag2;

	if (co->token == REX_TOKEN_EOF)
		return -1;
	while (1) {
		switch (co->token) {
		case REX_TOKEN_EOF:
		case '|':
		case ')':
			return 0;
		}
		if (rex_compiler_qfactor(co, rexdb) < 0)
			return -1;
		++nfactors;
		if (nfactors >= 2) {
			frag2 = FPOP(co);
			frag1 = FPOP(co);
			frag1 = rex_fragment_cat(rexdb, frag1, frag2);
			FPUSH(co, frag1);
		}
	}
	return 0;
}


static int rex_compiler_altexpression(rexcompiler_t *co, rexdb_t *rexdb)
{
	rexfragment_t *frag1;
	rexfragment_t *frag2;

	if (rex_compiler_catexpression(co, rexdb) < 0)
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
		if (rex_compiler_catexpression(co, rexdb) < 0)
			return -1;
		frag2 = FPOP(co);
		frag1 = FPOP(co);
		frag1 = rex_fragment_alt(rexdb, frag1, frag2);
		FPUSH(co, frag1);
	}
	return 0;
}


long rex_compiler_expression(rexcompiler_t *co, rexdb_t *rexdb, const char *str, unsigned int size, rexuserdata_t userdata)
{
	long curlen = r_array_length(rexdb->states);
	long i;
	long ret = -1L;
	rexfragment_t *frag1 = NULL, *frag2 = NULL;

	co->start = str;
	co->end = str + size;
	co->ptr = str;

	rex_compiler_getnbtok(co);
	if (rex_compiler_altexpression(co, rexdb) < 0)
		goto error;
	frag1 = FPOP(co);
	frag2 = rex_fragment_create(rex_db_getstate(rexdb, rex_db_createstate(rexdb, REX_STATETYPE_ACCEPT)));
	rex_state_setuserdata(REX_FRAG_STATE(frag2), userdata);
	frag1 = rex_fragment_cat(rexdb, frag1, frag2);
	ret = REX_FRAG_STATEUID(frag1);
	REX_FRAG_STATE(frag1)->type = REX_STATETYPE_START;
	rex_fragment_destroy(frag1);
	return ret;
error:
	if (curlen < r_array_length(rexdb->states)) {
		for (i = curlen; i < r_array_length(rexdb->states); i++)
			rex_state_destroy(rex_db_getstate(rexdb, i));
		r_array_setlength(rexdb->states, curlen);
	}
	while (FLEN(co)) {
		rex_fragment_destroy(FPOP(co));
	}
	return -1;
}


long rex_compiler_expression_s(rexcompiler_t *co, rexdb_t *rexdb, const char *str, rexuserdata_t userdata)
{
	return rex_compiler_expression(co, rexdb, str, r_strlen(str), userdata);
}


long rex_compiler_addexpression(rexcompiler_t *co, rexdb_t *rexdb, unsigned long prev, const char *str, unsigned int size, rexuserdata_t userdata)
{
	rexstate_t *sprev = NULL, *scur = NULL;
	long cur;
	if (r_array_empty(rexdb->states))
		prev = -1UL;
	cur = rex_compiler_expression(co, rexdb, str, size, userdata);
	if (cur < 0)
		return -1;
	sprev = rex_db_getstate(rexdb, prev);
	scur = rex_db_getstate(rexdb, cur);
	if (!sprev)
		return scur->uid;
	rex_state_addtransition_e_dst(sprev, scur);
	scur->type = REX_STATETYPE_NONE;
	sprev->type = REX_STATETYPE_START;
	return prev;
}


long rex_compiler_addexpression_s(rexcompiler_t *co, rexdb_t *rexdb, unsigned long prev, const char *str, rexuserdata_t userdata)
{
	return rex_compiler_addexpression(co, rexdb, prev, str, r_strlen(str), userdata);
}
