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

#ifndef _RSTRING_H_
#define _RSTRING_H_


#include "rtypes.h"
#include "rlib/robject.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "rlib/rref.h"



typedef struct rstr_s {
	rchar *str;
	rsize_t size;
} rstr_t;


rinteger r_strcmp(const rchar *s1, const rchar *s2);
rinteger r_strncmp(const rchar *s1, const rchar *s2, rsize_t n);
rboolean r_stringncmp(const rchar *str, const rchar *s2, rsize_t n);
rsize_t r_strlen(const rchar *s);
rchar *r_strstr(const rchar *haystack, const rchar *needle);
rchar *r_strchr(const rchar *s, rinteger c);
rchar *r_strdup(const rchar *s);
rchar *r_strndup(const rchar *s, rsize_t size);
rchar *r_strcpy(rchar *dest, const rchar *src);
rchar *r_strncpy(rchar *dest, const rchar *src, rsize_t n);
rchar *r_strcat(rchar *dest, const rchar *src);
rchar *r_strncat(rchar *dest, const rchar *src, ruinteger n);
rinteger r_snprintf(rchar *str, ruinteger size, const rchar *format, ...);
rinteger r_printf(const rchar *format, ...);

rulong r_strtoul(const rchar *nptr, rchar **endptr, rinteger base);
rlong r_strtol(const rchar *s, rchar **endptr, rinteger base);
rdouble r_strtod(const rchar *s, rchar **endptr);

rstr_t *r_rstrdup(const rchar *s, ruinteger size);


typedef struct rstring_s {
	robject_t obj;
	rstr_t s;
} rstring_t;

#define R_STRING2ANSI(__p__) ((rstring_t *)(__p__))->s.str
#define R_STRING2RSTR(__p__) &((rstring_t *)(__p__))->s
#define r_string_empty(__p__) (!((__p__) && ((rstring_t *)(__p__))->s.size)) ? 1 : 0
rstring_t *r_string_create();
rstring_t *r_string_create_from_ansistr(const rchar *str);
rstring_t *r_string_create_from_double(rdouble d);
rstring_t *r_string_create_from_long(rlong l);
rstring_t *r_string_create_strsize(const char *str, ruinteger size);
rstring_t *r_string_create_from_rstr(const rstr_t *str);
void r_string_destroy(rstring_t *string);
robject_t *r_string_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
void r_string_assign(rstring_t *string, const rstr_t *str);
void r_string_cat(rstring_t *string, const rstr_t *str);
const char *r_string_ansi(const rstring_t *str);
/*
 * Virtual methods implementation
 */
void r_string_cleanup(robject_t *obj);
robject_t *r_string_copy(const robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
