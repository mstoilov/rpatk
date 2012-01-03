#include <ctype.h>
#include <stdlib.h>
#include "rlib/rmem.h"
#include "rex/rexcompiler.h"

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


rexfragment_t *rex_compiler_expression(rexcompiler_t *co, const char *str, unsigned int size, void *accdata)
{

	return NULL;
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

	while (c != REX_TOKEN_EOF && rex_compiler_isspace(c))
		c = rex_compiler_getchar(co);
	co->token = c;
	return c;
}


