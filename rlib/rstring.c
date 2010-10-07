#include <stdlib.h>
#include <string.h>

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
		r_memset(d, allocsize, 0);
		d->size = size;
		d->str = (rchar*)&d[1];
		r_memcpy((rchar*)d->str, s, size);
	}
	return d;
}


static void r_refstub_destroy(rref_t *ref)
{
	r_string_destroy((rstring_t*)ref);
}


static rref_t *r_refstub_copy(const rref_t *ptr)
{
	return (rref_t*) r_string_copy((const rstring_t*)ptr);
}


rstring_t *r_string_create()
{
	rstring_t *string;
	if ((string = (rstring_t*)r_malloc(sizeof(*string))) == NULL)
		return NULL;
	if (!r_string_init(string)) {
		r_string_destroy(string);
		return NULL;
	}
	r_ref_init(&string->ref, 1, RREF_TYPE_NONE, r_refstub_destroy, r_refstub_copy);
	return string;

}


rstring_t *r_string_init(rstring_t *string)
{
	r_memset(string, 0, sizeof(*string));
	return string;
}


void r_string_destroy(rstring_t *string)
{
	r_string_cleanup(string);
	r_free(string);
}


void r_string_cleanup(rstring_t *string)
{
	if (string) {
		r_free(string->s.str);
		r_memset(&string->s, 0, sizeof(rstr_t));
	}
}


void r_string_assign(rstring_t *string, const rstr_t *str)
{
	r_string_cleanup(string);
	if (str->size) {
		string->s.str = (rchar*)r_malloc(str->size + 1);
		if (!string->s.str)
			return;
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


rstring_t *r_string_copy(const rstring_t *srcString)
{
	rstring_t *string = r_string_create();
	if (string && srcString) {
		r_string_assign(string, &srcString->s);
	}
	return string;
}


rstring_t *r_string_create_from_rstr(const rstr_t *str)
{
	rstring_t *string = r_string_create();
	if (string)
		r_string_assign(string, str);
	return string;
}
