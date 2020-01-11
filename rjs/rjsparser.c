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

#include "rlib/rmem.h"
#include "rjs/rjsrules.h"
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
		long line = 1, i;
		const char *ptr = rjs_rules_get();
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
	if (!parser->dbex) {
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


long rjs_parser_offset2line(const char *script, long offset)
{
	long line = 1;
	const char *ptr;

	for (line = 1, ptr = script + offset; ptr >= script; --ptr) {
		if (*ptr == '\n' && ptr != script + offset)
			line += 1;
	}

	return line;
}


long rjs_parser_offset2lineoffset(const char *script, long offset)
{
	long lineoffset = 0;
	const char *ptr;

	for (lineoffset = 0, ptr = script + offset; ptr > script; --ptr) {
		if (*ptr == '\n' && ptr != script + offset)
			break;
		lineoffset += 1;
	}
	return offset - lineoffset;
}


long rjs_parser_exec(rjs_parser_t *parser, const char *script, unsigned long size, rarray_t *ast, rjs_error_t *error)
{
	long res = 0;
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
