#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"

#define FPUSH(__co__, __fragment__) do { r_array_push((__co__)->stack, __fragment__, rexfragment_t*); } while (0)
#define FPOP(__co__) r_array_pop((__co__)->stack, rexfragment_t*)

static int rex_compiler_catexpression(rexcompiler_t *co);
static int rex_compiler_altexpression(rexcompiler_t *co);

enum {
	REX_TOKEN_ERR = -1,
	REX_TOKEN_EOF = -2,
	REX_TOKEN_SPACE = -4,
	REX_TOKEN_NEWLINE = -5,

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
	int c = REX_TOKEN_EOF;

	if (co->ptr < co->end)
		c = *co->ptr++;
	return c;
}


static int rex_compiler_peekchar(rexcompiler_t *co)
{
	int c = REX_TOKEN_EOF;

	if (co->ptr < co->end)
		c = *co->ptr;
	return c;
}


static int rex_compiler_gettok(rexcompiler_t *co)
{
	int c = rex_compiler_getchar(co);

	if (c == REX_TOKEN_EOF) {
		co->tokenstr[0] = '\0';
		co->token = REX_TOKEN_EOF;
		return co->token;
	} else if (rex_compiler_isspace(c)) {
		co->tokenstr[0] = c;
		co->tokenstr[1] = '\0';
		co->token = REX_TOKEN_SPACE;
	} else {
		co->tokenstr[0] = c;
		co->tokenstr[1] = '\0';
		co->token = c;
	}
	return co->token;
}


int rex_compiler_charclass(rexcompiler_t *co)
{
	int negative = 0;
	int low;
	int high;
	rexfragment_t *frag = rex_fragment_create(co->db);

	FPUSH(co, frag);
	rex_compiler_gettok(co);
	if (co->token == '^') {
		negative = 1;
		rex_compiler_gettok(co);
	}

	while (1) {
		switch (co->token) {
		case '[':
		case ']':
		case '-':
			return -1;
			break;
		}
		high = low = co->token;
		rex_compiler_gettok(co);
		if (co->token == '-') {
			high = rex_compiler_gettok(co);
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
			rex_compiler_gettok(co); /* eat it */
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
		rex_compiler_gettok(co);		/* eat '(' */
		rex_compiler_altexpression(co);
		while (co->token == REX_TOKEN_SPACE)
			rex_compiler_gettok(co);
		if (co->token != ')')
			return -1;
		rex_compiler_gettok(co);		/* eat ')' */
		return 0;
	} else {
		rexfragment_t *frag;
		frag = rex_fragment_create_singletransition(co->db, co->token);
//		rex_state_dump(rex_db_getstate(co->db, frag->start));
		FPUSH(co, frag);
		rex_compiler_gettok(co);
		return 0;
	}

	return -1;
}


static int rex_compiler_qfactor(rexcompiler_t *co)
{
	const char *begin = co->ptr - 1;
	rexfragment_t *frag;

	while (co->token == REX_TOKEN_SPACE)
		rex_compiler_gettok(co);
	if (strchr("*?+])", co->token)) {
		/*
		 * Unexpected char.
		 */
		return -1;
	}
	if (rex_compiler_factor(co) < 0) {
		return -1;
	}
	while (co->token == REX_TOKEN_SPACE)
		rex_compiler_gettok(co);
	switch (co->token) {
	case '*':
		rex_compiler_gettok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_mop(frag);
		FPUSH(co, frag);
		break;
	case '?':
		rex_compiler_gettok(co); /* Eat it */
		frag = FPOP(co);
		frag = rex_fragment_opt(frag);
		FPUSH(co, frag);
		break;
	case '+':
		rex_compiler_gettok(co); /* Eat it */
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
			return 0;
		case REX_TOKEN_SPACE:
			rex_compiler_gettok(co);
			continue;
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
	while (co->token == REX_TOKEN_SPACE)
		rex_compiler_gettok(co);

	while (co->token == '|') {
		rex_compiler_gettok(co); /* Eat '|' */
		while (co->token == REX_TOKEN_SPACE)
			rex_compiler_gettok(co);
		if (rex_compiler_catexpression(co) < 0)
			return -1;
		while (co->token == REX_TOKEN_SPACE)
			rex_compiler_gettok(co);
		frag2 = FPOP(co);
		frag1 = FPOP(co);
		frag1 = rex_fragment_alt(frag1, frag2);
		FPUSH(co, frag1);
	}
	return 0;
}



rexfragment_t *rex_compiler_expression(rexcompiler_t *co, const char *str, unsigned int size, void *accdata)
{
	rexfragment_t *frag = NULL;
	co->start = str;
	co->end = str + size;
	co->ptr = str;

	rex_compiler_gettok(co);
	rex_compiler_altexpression(co);
	frag = FPOP(co);
	return frag;
}


rexfragment_t *rex_compiler_expression_s(rexcompiler_t *co, const char *str, void *accdata)
{
	return rex_compiler_expression(co, str, r_strlen(str), accdata);
}

