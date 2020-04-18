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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "rlib/rstring.h"
#include "rlib/rmem.h"

rstr_t *r_rstrdup(const char *s, unsigned int size)
{
	unsigned long allocsize = sizeof(rstr_t) + size + sizeof(char);
	rstr_t *d = (rstr_t*)r_malloc(allocsize);
	if (d) {
		r_memset(d, 0, allocsize);
		d->size = size;
		d->str = (char*)&d[1];
		if (s)
			r_memcpy((char*)d->str, s, size);
	}
	return d;
}

void r_rstr_free(rstr_t* rstr)
{
	r_free(rstr);
}

int r_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}


int r_strncmp(const char *s1, const char *s2, size_t n)
{
	int ret = strncmp(s1, s2, (size_t)n);
	return ret;
}


rboolean r_stringncmp(const char *str, const char *s2, size_t n)
{
	if (r_strlen(str) == n && r_strncmp(str, s2, (size_t)n) == 0)
		return 1;
	return 0;
}


size_t r_strlen(const char *s)
{
	return s ? (size_t)strlen(s) : 0;
}


char *r_strstr(const char *haystack, const char *needle)
{
	return strstr(haystack, needle);
}


char *r_strchr(const char *s, int c)
{
	return strchr(s, c);
}


char *r_strtrimr(char *s, const char *chrs)
{
	int i;
	for (i = r_strlen(s) - 1; i >= 0 && r_strchr(chrs, s[i]) != NULL; i--)
		s[i] = '\0';
	return s;
}


char *r_strcpy(char *dest, const char *src)
{
	return (char*) strcpy(dest, src);
}


char *r_strncpy(char *dest, const char *src, size_t n)
{
	return (char*) strncpy(dest, src, n);
}


char *r_strdup(const char *s)
{
	size_t size = r_strlen(s);
	char *dupstr = (char*)r_zmalloc(size + 1);

	if (dupstr) {
		r_strcpy(dupstr, s);
	}
	return dupstr;
}


char *r_strndup(const char *s, size_t size)
{
	char *dupstr = (char*)r_zmalloc(size + 1);

	if (dupstr) {
		r_strncpy(dupstr, s, size);
	}
	return dupstr;
}


int r_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return vsnprintf(str, size, format, ap);
}


char *r_strcat(char *dest, const char *src)
{
	return strcat(dest, src);
}


char *r_strncat(char *dest, const char *src, size_t n)
{
	return strncat(dest, src, n);
}


int r_snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = r_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}


int r_printf(const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = vfprintf(stdout, format, ap);
	va_end(ap);
	return ret;
}


size_t r_strtoul(const char *nptr, char **endptr, int base)
{
	return strtoul(nptr, endptr,base);
}


long r_strtol(const char *s, char **endptr, int base)
{
	return strtol(s, endptr, base);
}


double r_strtod(const char *s, char **endptr)
{
	return strtod(s, endptr);
}

robject_t *r_string_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	rstring_t *string = (rstring_t*)obj;
	r_object_init(obj, type, cleanup, copy);
	string->str = NULL;
	string->size = 0;

	return obj;
}

robject_t *r_string_init_from_str_len(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy, const char *str, size_t len)
{
	rstring_t *string = (rstring_t*)obj;
	r_string_init(obj, type, cleanup, copy);
	if (len == 0 || str == NULL) {
		/*
		 * Create empty string
		 */
		string->str = r_zmalloc(1);
	} else {
		string->str = r_zmalloc(len + 1);
		r_strncpy(string->str, str, len);
		string->size = r_strlen(string->str);
	}
	return obj;
}

static void r_string_cleanup(robject_t *obj)
{
	rstring_t *string = (rstring_t*)obj;
	if (string)
		r_free(string->str);
	r_object_cleanup(obj);
}

static robject_t *r_string_copy(const robject_t *obj)
{
	const rstring_t *src = (const rstring_t *)obj;
	return (robject_t*)r_string_create_from_str_len(src->str, src->size);
}

void r_string_destroy(rstring_t *string)
{
	r_object_destroy((robject_t*)string);
}

rstring_t *r_string_create_from_ansi(const char* s)
{
	rstring_t *string;
	string = (rstring_t*)r_object_create(sizeof(*string));
	r_string_init_from_str_len(&string->obj, R_OBJECT_STRING, r_string_cleanup, r_string_copy, s, r_strlen(s));
	return string;
}

rstring_t *r_string_create_from_str_len(const char* s, size_t len)
{
	rstring_t *string;
	string = (rstring_t*)r_object_create(sizeof(*string));
	r_string_init_from_str_len(&string->obj, R_OBJECT_STRING, r_string_cleanup, r_string_copy, s, len);
	return string;
}

rstring_t *r_string_create_from_string(const rstring_t* s)
{
	rstring_t *string;
	string = (rstring_t*)r_object_create(sizeof(*string));
	r_string_init_from_str_len(&string->obj, R_OBJECT_STRING, r_string_cleanup, r_string_copy, s->str, s->size);
	return string;
}

/*
 * Concatenate up to size chars from str2 to str1
 */
void r_string_cat_len(rstring_t *str1, const char *str2, size_t str2len)
{
	if (str2 && str2len) {
		str1->str = r_realloc(str1->str, str1->size + str2len + 1);
		r_memset(str1->str + str1->size, 0, str2len + 1);
		r_strncpy(str1->str + str1->size, str2, str2len);
		str1->size += r_strlen(str1->str + str1->size);
	}
}

/*
 * Concatenate up to size chars from str2 to str1
 */
rstring_t *r_string_cat(rstring_t *str1, rstring_t *str2)
{
	rstring_t *str = r_string_create_from_string(str1);
	if (str)
		r_string_cat_len(str, str2->str, str2->size);
	return str;
}


rstring_t *r_string_create_from_double(double d)
{
	char temp[128];
	int size;

	r_memset(temp, 0, sizeof(temp));
	size = r_snprintf(temp, sizeof(temp) - 1, "%f", d);
	if (size > 0 && size < (int)sizeof(temp)) {
		return r_string_create_from_str_len(temp, r_strlen(temp));
	}
	return NULL;
}


rstring_t *r_string_create_from_int(rint64 l)
{
	char temp[128];
	int size;

	r_memset(temp, 0, sizeof(temp));
	size = r_snprintf(temp, sizeof(temp) - 1, "%lld", l);
	if (size > 0 && size < (int)sizeof(temp)) {
		return r_string_create_from_str_len(temp, size);
	}
	return NULL;
}

