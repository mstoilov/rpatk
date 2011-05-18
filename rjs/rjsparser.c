#include "rmem.h"
#include "rjsrules.h"
#include "rjsparser.h"


void rjs_parser_dbex_error(rjs_parser_t *parser)
{
	rpa_errinfo_t err;
	r_memset(&err, 0, sizeof(err));
	rpa_dbex_lasterrorinfo(parser->dbex, &err);
	r_printf("Failed to load RPA rules");
	if (err.mask & RPA_ERRINFO_CODE) {
		r_printf(", Error Code: %ld", err.code);
	}
	if (err.mask & RPA_ERRINFO_OFFSET) {
		rlong line = 1, i;
		const rchar *ptr = rjs_rules_get();
		for (i = 0; i < err.offset; i++) {
			if (ptr[i] == '\n')
				line += 1;
		}
		r_printf(", Offset: %ld, Line: %ld", err.offset, line);
	}
	if (err.mask & RPA_ERRINFO_NAME) {
		r_printf(", Rule Name: %s", err.name);
	}
	r_printf("\n");
}


rjs_parser_t *rjs_parser_create()
{
	rjs_parser_t *parser = (rjs_parser_t *) r_zmalloc(sizeof(*parser));

	parser->dbex = rpa_dbex_create();
	if (!parser) {
		rjs_parser_destroy(parser);
		return NULL;
	}
	rpa_dbex_open(parser->dbex);
	if (rpa_dbex_load(parser->dbex, rjs_rules_get(), rjs_rules_size()) < 0) {
		rjs_parser_dbex_error(parser);
		rjs_parser_destroy(parser);
		return NULL;
	}
	rpa_dbex_close(parser->dbex);
	if (rpa_dbex_compile(parser->dbex) < 0) {
		rjs_parser_dbex_error(parser);
		rjs_parser_destroy(parser);
		return NULL;
	}
	return parser;
}


void rjs_parser_destroy(rjs_parser_t *parser)
{
	if (parser) {
		rpa_dbex_destroy(parser->dbex);
		r_free(parser);
	}
}


rlong rjs_parser_exec(rjs_parser_t *parser, const rchar *script, rsize_t size, rarray_t **ast)
{
	rlong res = 0;
	rpastat_t *stat = rpa_stat_create(parser->dbex, 4096);
	res = rpa_stat_parse(stat, rpa_dbex_last(parser->dbex), script, script, script + size, ast);
	rpa_stat_destroy(stat);
	return res;
}
