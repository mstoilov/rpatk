#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rlib/rutf.h"
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"

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
	return co;
}


void rex_compiler_destroy(rexcompiler_t *co)
{
	if (co) {
		r_array_destroy(co->stack);
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


int rex_compiler_charclass(rexcompiler_t *co)
{
	int negative = 0;
	int low;
	int high;
	rexfragment_t *frag = rex_fragment_create(co->db);

	FPUSH(co, frag);
	rex_compiler_getnstok(co);
	if (co->token == '^') {
		negative = 1;
		rex_compiler_gettok(co);
	}

	while (1) {
		if (co->token == '[' || co->token == ']' || co->token == '-')
			return -1;
		rex_compiler_adjustescapedtoken(co);
		high = low = co->token;
		rex_compiler_gettok(co);
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
			rex_compiler_adjustescapedtoken(co);
			high = co->token;
			rex_compiler_gettok(co);
		}
		if (low == high) {
			rex_fragment_singletransition(frag, low);
		} else if (low < high) {
			rex_fragment_rangetransition(frag, low, high);
		} else {
			rex_fragment_rangetransition(frag, high, low);
		}
		if (co->token == ']') {
			rex_compiler_getnstok(co); /* eat it */
			break;
		}
	}

	return 0;
}


static int rex_compiler_factor(rexcompiler_t *co)
{
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
		rexfragment_t *frag;
		frag = rex_fragment_create(co->db);
		rex_fragment_rangetransition(frag, 0, '\n' - 1);
		rex_fragment_rangetransition(frag, '\n' + 1, (unsigned int)-1);
		FPUSH(co, frag);
		rex_compiler_getnstok(co);
		return 0;
	} else {
		rexfragment_t *frag;
		rex_compiler_adjustescapedtoken(co);
		frag = rex_fragment_create_singletransition(co->db, co->token);
		FPUSH(co, frag);
		rex_compiler_getnstok(co);
		return 0;
	}

	return -1;
}


static int rex_compiler_qfactor(rexcompiler_t *co)
{
	const char *begin = co->ptr - 1;
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
		frag = rex_fragment_mop(frag);
		FPUSH(co, frag);
		break;
	case '?':
		rex_compiler_getnstok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_opt(frag);
		FPUSH(co, frag);
		break;
	case '+':
		rex_compiler_getnstok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mul(frag);
		FPUSH(co, frag);
		break;
	}
#if 0
	if (co->ptr - 1 > begin) {
		fprintf(stdout, "qfactor: ");
		fwrite(begin, 1, co->ptr - begin - 1, stdout);
		fprintf(stdout, "\n");
	}
#endif
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
			frag1 = rex_fragment_cat(frag1, frag2);
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
		if (rex_compiler_catexpression(co) < 0)
			return -1;
		frag2 = FPOP(co);
		frag1 = FPOP(co);
		frag1 = rex_fragment_alt(frag1, frag2);
		FPUSH(co, frag1);
	}
	return 0;
}



rexfragment_t *rex_compiler_expression(rexcompiler_t *co, const char *str, unsigned int size, void *accdata)
{
	long curlen = r_array_length(co->db->states);
	long i;
	rexfragment_t *frag = NULL;

	co->start = str;
	co->end = str + size;
	co->ptr = str;

	rex_compiler_getnstok(co);
	if (rex_compiler_altexpression(co) < 0)
		goto error;
	frag = FPOP(co);
	rex_fragment_set_endstatetype(frag, REX_STATETYPE_ACCEPT);
	rex_state_setuserdata(rex_fragment_endstate(frag), accdata);
	return frag;
error:
	if (curlen < r_array_length(co->db->states)) {
		for (i = curlen; i < r_array_length(co->db->states); i++)
			rex_state_destroy(rex_db_getstate(co->db, i));
		r_array_setlength(co->db->states, curlen);
	}
	while (FLEN(co)) {
		rex_fragment_destroy(FPOP(co));
	}
	return NULL;
}


rexfragment_t *rex_compiler_expression_s(rexcompiler_t *co, const char *str, void *accdata)
{
	return rex_compiler_expression(co, str, r_strlen(str), accdata);
}


rexfragment_t *rex_compiler_addexpression(rexcompiler_t *co, rexfragment_t *frag1, const char *str, unsigned int size, void *accdata)
{
	rexfragment_t *frag2 = rex_compiler_expression(co, str, size, accdata);

	if (!frag2)
		return NULL;
	if (!frag1)
		return frag2;
	frag1 = rex_fragment_alt(frag1, frag2);
	return frag1;
}


rexfragment_t *rex_compiler_addexpression_s(rexcompiler_t *co, rexfragment_t *frag1, const char *str, void *accdata)
{
	return rex_compiler_addexpression(co, frag1, str, r_strlen(str), accdata);
}
