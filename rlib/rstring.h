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
	char *str;
	rsize_t size;
} rstr_t;


int r_strcmp(const char *s1, const char *s2);
int r_strncmp(const char *s1, const char *s2, rsize_t n);
rboolean r_stringncmp(const char *str, const char *s2, rsize_t n);
rsize_t r_strlen(const char *s);
char *r_strstr(const char *haystack, const char *needle);
char *r_strchr(const char *s, int c);
char *r_strdup(const char *s);
char *r_strndup(const char *s, rsize_t size);
char *r_strcpy(char *dest, const char *src);
char *r_strncpy(char *dest, const char *src, rsize_t n);
char *r_strcat(char *dest, const char *src);
char *r_strncat(char *dest, const char *src, unsigned int n);
int r_snprintf(char *str, unsigned int size, const char *format, ...);
int r_printf(const char *format, ...);

unsigned long r_strtoul(const char *nptr, char **endptr, int base);
long r_strtol(const char *s, char **endptr, int base);
double r_strtod(const char *s, char **endptr);

rstr_t *r_rstrdup(const char *s, unsigned int size);


typedef struct rstring_s {
	robject_t obj;
	rstr_t s;
} rstring_t;

#define R_STRING2ANSI(__p__) ((rstring_t *)(__p__))->s.str
#define R_STRING2RSTR(__p__) &((rstring_t *)(__p__))->s
#define r_string_empty(__p__) (!((__p__) && ((rstring_t *)(__p__))->s.size)) ? 1 : 0
rstring_t *r_string_create();
rstring_t *r_string_create_from_ansistr(const char *str);
rstring_t *r_string_create_from_double(double d);
rstring_t *r_string_create_from_signed(long l);
rstring_t *r_string_create_strsize(const char *str, unsigned int size);
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
