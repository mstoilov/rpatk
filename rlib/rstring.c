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


rstring_t *r_stringdup(const rchar *s, ruint size)
{
	rsize_t allocsize = sizeof(rstring_t) + size + sizeof(rchar);
	rstring_t *d = (rstring_t*)r_malloc(allocsize);
	if (d) {
		r_memset(d, allocsize, 0);
		d->size = size;
		d->str = (rchar*)&d[1];
		r_memcpy((rchar*)d->str, s, size);
	}
	return d;
}
