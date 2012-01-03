#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"

enum {
	REX_TOKEN_ERR = -1,
	REX_TOKEN_EOF = -2,
	REX_TOKEN_REGEXCHAR = -3,

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


static int rex_compiler_gettok(rexcompiler_t *co)
{
	int c = rex_compiler_getchar(co);

	if (c == REX_TOKEN_EOF) {
		co->tokenstr[0] = '\0';
		co->token = REX_TOKEN_EOF;
		return co->token;
	}
	while (rex_compiler_isspace(c))
		c = rex_compiler_getchar(co);
	if (strchr("*?+[]()|.", c)) {
		co->tokenstr[0] = c;
		co->tokenstr[1] = '\0';
		co->token = c;
	} else {
		co->tokenstr[0] = c;
		co->tokenstr[1] = '\0';
		co->token = REX_TOKEN_REGEXCHAR;
	}
	return co->token;
}


rexfragment_t *rex_compiler_expression(rexcompiler_t *co, const char *str, unsigned int size, void *accdata)
{
	co->start = str;
	co->end = str + size;
	co->ptr = str;

	while (rex_compiler_gettok(co) != REX_TOKEN_EOF) {
		fprintf(stdout, "token = %d (%s)\n", co->token, co->tokenstr);
	}
	return NULL;
}


rexfragment_t *rex_compiler_expression_s(rexcompiler_t *co, const char *str, void *accdata)
{
	return rex_compiler_expression(co, str, r_strlen(str), accdata);
}

