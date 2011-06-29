#include "rlib/rmem.h"
#include "rjs/linux/rjsrules.h"
#include "rjs/rjsparser.h"
#include "rjs/rjserror.h"


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


rlong rjs_parser_offset2line(const rchar *script, rlong offset)
{
	rlong line = 1;
	const rchar *ptr;

	for (line = 1, ptr = script + offset; ptr >= script; --ptr) {
		if (*ptr == '\n' && ptr != script + offset)
			line += 1;
	}

	return line;
}


rlong rjs_parser_offset2lineoffset(const rchar *script, rlong offset)
{
	rlong lineoffset = 0;
	const rchar *ptr;

	for (lineoffset = 0, ptr = script + offset; ptr > script; --ptr) {
		if (*ptr == '\n' && ptr != script + offset)
			break;
		lineoffset += 1;
	}
	return offset - lineoffset;
}


rlong rjs_parser_exec(rjs_parser_t *parser, const rchar *script, rsize_t size, rarray_t *ast, rjs_error_t *error)
{
	rlong res = 0;
	rpastat_t *stat = rpa_stat_create(parser->dbex, 4096);
	res = rpa_stat_parse(stat, rpa_dbex_last(parser->dbex), RPA_ENCODING_UTF8, script, script, script + size, ast);
	if (res < 0 && error) {
		rpa_errinfo_t rpaerror;
		rpa_stat_lasterrorinfo(stat, &rpaerror);
		if (rpaerror.code == RPA_E_RULEABORT) {
			error->type = RJS_ERRORTYPE_SYNTAX;
			error->error = RJS_ERROR_SYNTAX;
			error->offset = rpaerror.offset;
			error->line = rjs_parser_offset2line(script, error->offset);
			error->lineoffset = rjs_parser_offset2lineoffset(script, error->offset);
		}
	}
	rpa_stat_destroy(stat);
	return res;
}
