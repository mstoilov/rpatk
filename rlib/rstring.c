#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "rstring.h"
#include "rmem.h"


rint r_strcmp(const rchar *s1, const rchar *s2)
{
	return strcmp(s1, s2);
}


rint r_strncmp(const rchar *s1, const rchar *s2, rsize_t n)
{
	int ret = strncmp(s1, s2, (size_t)n);
	return ret;
}


rsize_t r_strlen(const rchar *s)
{
	return strlen(s);
}


rchar *r_strstr(const rchar *haystack, const rchar *needle)
{
	return strstr(haystack, needle);
}


rchar *r_strchr(const rchar *s, rint c)
{
	return strchr(s, c);
}


rchar *r_strcpy(rchar *dest, const rchar *src)
{
	return (rchar*) strcpy(dest, src);
}


rchar *r_strncpy(rchar *dest, const rchar *src, rsize_t n)
{
	return (rchar*) strncpy(dest, src, n);
}


rchar *r_strdup(const rchar *s)
{
	rsize_t size = r_strlen(s);
	rchar *dupstr = (rchar*)r_zmalloc(size + 1);

	if (dupstr) {
		r_strcpy(dupstr, s);
	}
	return dupstr;
}


rchar *r_strndup(const rchar *s, rsize_t size)
{
	rchar *dupstr = (rchar*)r_zmalloc(size + 1);

	if (dupstr) {
		r_strncpy(dupstr, s, size);
	}
	return dupstr;
}


rstr_t *r_rstrdup(const rchar *s, ruint size)
{
	rsize_t allocsize = sizeof(rstr_t) + size + sizeof(rchar);
	rstr_t *d = (rstr_t*)r_malloc(allocsize);
	if (d) {
		r_memset(d, 0, allocsize);
		d->size = size;
		d->str = (rchar*)&d[1];
		r_memcpy((rchar*)d->str, s, size);
	}
	return d;
}


rint r_vsnprintf(rchar *str, ruint size, const rchar *format, va_list ap)
{
	return vsnprintf(str, size, format, ap);
}


rchar *r_strcat(rchar *dest, const rchar *src)
{
	return strcat(dest, src);
}


rchar *r_strncat(rchar *dest, const rchar *src, ruint n)
{
	return strncat(dest, src, n);
}


rint r_snprintf(rchar *str, ruint size, const rchar *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = r_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}


rint r_printf(const rchar *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = vfprintf(stdout, format, ap);
	va_end(ap);
	return ret;
}


rulong r_strtoul(const rchar *nptr, rchar **endptr, rint base)
{
	return strtoul(nptr, endptr,base);
}


rlong r_strtol(const rchar *s, rchar **endptr, rint base)
{
	return strtol(s, endptr, base);
}


rdouble r_strtod(const rchar *s, rchar **endptr)
{
	return strtod(s, endptr);
}


robject_t *r_string_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy)
{
	rstring_t *string = (rstring_t*)obj;
	r_object_init(obj, type, cleanup, copy);
	r_memset(&string->s, 0, sizeof(string->s));
	return obj;
}


void r_string_cleanup(robject_t *obj)
{
	rstring_t *string = (rstring_t*)obj;
	if (string) {
		r_free(string->s.str);
		r_memset(&string->s, 0, sizeof(string->s));
	}
	r_object_cleanup(obj);
}


rstring_t *r_string_create()
{
	rstring_t *string;
	string = (rstring_t*)r_object_create(sizeof(*string));
	r_string_init(&string->obj, R_OBJECT_STRING, r_string_cleanup, r_string_copy);
	return string;

}


void r_string_assign(rstring_t *string, const rstr_t *str)
{
	if (str && str->size) {
		r_free(string->s.str);
		string->s.str = (rchar*)r_malloc(str->size + 1);
		if (!string->s.str)
			return;
		r_memset(string->s.str, 0, str->size);
		r_memcpy(string->s.str, str->str, str->size);
		string->s.size = str->size;
	}
}


void r_string_cat(rstring_t *string, const rstr_t *str)
{
	if (str && str->size) {
		string->s.str = r_realloc(string->s.str, string->s.size + str->size + 1);
		r_memset(string->s.str + string->s.size, 0, str->size + 1);
		r_strncpy(string->s.str + string->s.size, str->str, str->size);
		string->s.size += str->size;
	}
}


robject_t *r_string_copy(const robject_t *obj)
{
	const rstring_t *srcString = (const rstring_t *)obj;
	return (robject_t*)r_string_create_from_rstr(&srcString->s);
}


rstring_t *r_string_create_from_rstr(const rstr_t *str)
{
	rstring_t *string = r_string_create();
	if (string)
		r_string_assign(string, str);
	return string;
}


rstring_t *r_string_create_from_ansistr(const rchar *str)
{
	rstr_t rstr;

	r_memset(&rstr, 0, sizeof(rstr));
	if (str) {
		rstr.str = (rchar*)str;
		rstr.size = r_strlen(str);
	}
	return r_string_create_from_rstr(&rstr);
}


rstring_t *r_string_create_strsize(const rchar *str, ruint size)
{
	rstr_t rstr;

	r_memset(&rstr, 0, sizeof(rstr));
	if (str) {
		rstr.str = (rchar*)str;
		rstr.size = size;
	}
	return r_string_create_from_rstr(&rstr);
}
