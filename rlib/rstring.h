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

#ifndef _RSTRING_H_
#define _RSTRING_H_


#include "rtypes.h"
#include "rlib/robject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rstr_s {
	char *str;
	unsigned long size;
} rstr_t;

rstr_t *r_rstrdup(const char *s, unsigned int size);
void r_rstr_free(rstr_t* rstr);

int r_strcmp(const char *s1, const char *s2);
int r_strncmp(const char *s1, const char *s2, size_t n);
rboolean r_stringncmp(const char *str, const char *s2, size_t n);
size_t r_strlen(const char *s);
char *r_strstr(const char *haystack, const char *needle);
char *r_strchr(const char *s, int c);
char *r_strdup(const char *s);
char *r_strndup(const char *s, size_t size);
char *r_strcpy(char *dest, const char *src);
char *r_strncpy(char *dest, const char *src, size_t n);
char *r_strcat(char *dest, const char *src);
char *r_strncat(char *dest, const char *src, size_t n);
int r_snprintf(char *str, size_t size, const char *format, ...);
int r_printf(const char *format, ...);
size_t r_strtoul(const char *nptr, char **endptr, int base);
long r_strtol(const char *s, char **endptr, int base);
double r_strtod(const char *s, char **endptr);


typedef struct rstring_s {
	robject_t obj;
	char *str;
	size_t size;
} rstring_t;

#define R_STRING2ANSI(__p__) ((rstring_t *)(__p__))->str
#define r_string_empty(__p__) (!((__p__) && ((rstring_t *)(__p__))->size)) ? 1 : 0
rstring_t *r_string_create_from_ansi(const char* s);
rstring_t *r_string_create_from_str_len(const char* s, size_t len);
rstring_t *r_string_create_from_string(const rstring_t* s);
rstring_t *r_string_create_from_double(double d);
rstring_t *r_string_create_from_int(rint64 l);
void r_string_destroy(rstring_t *string);
robject_t *r_string_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
robject_t *r_string_init_from_str_len(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, const char *str, size_t len);
rstring_t *r_string_cat(rstring_t *str1, rstring_t *str2);
void r_string_cat_len(rstring_t *string, const char *str, size_t size);


#ifdef __cplusplus
}
#endif

#endif
