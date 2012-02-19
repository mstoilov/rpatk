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

#ifndef _RJSPARSER_H_
#define _RJSPARSER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rvm/rvmcpu.h"
#include "rpa/rpa.h"
#include "rjs/rjserror.h"


typedef struct rjs_parser_s {
	rpadbex_t *dbex;

} rjs_parser_t;


rjs_parser_t *rjs_parser_create();
void rjs_parser_destroy(rjs_parser_t *parser);
long rjs_parser_exec(rjs_parser_t *parser, const char *script, unsigned long size, rarray_t *ast, rjs_error_t *error);
long rjs_parser_offset2line(const char *script, long offset);
long rjs_parser_offset2lineoffset(const char *script, long offset);


#ifdef __cplusplus
}
#endif

#endif
