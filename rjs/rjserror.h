/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
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

#ifndef _RJSERROR_H_
#define _RJSERROR_H_


#define RJS_ERRORTYPE_NONE 0
#define RJS_ERRORTYPE_SYNTAX 1
#define RJS_ERRORTYPE_RUNTIME 2


#define RJS_ERROR_NONE 0
#define RJS_ERROR_UNDEFINED 1
#define RJS_ERROR_SYNTAX 2
#define RJS_ERROR_NOTAFUNCTION 3
#define RJS_ERROR_NOTAFUNCTIONCALL 4
#define RJS_ERROR_NOTALOOP 5
#define RJS_ERROR_NOTAIFSTATEMENT 6


typedef struct rjs_error_s {
	rlong type;
	rlong error;
	rlong offset;
	rlong size;
	rlong line;
	rlong lineoffset;
	const rchar *script;
} rjs_error_t;


#endif /* RJSERROR_H_ */
