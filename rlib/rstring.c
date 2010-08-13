#include <stdlib.h>
#include <string.h>

#include "rstring.h"

rint r_strcmp(const rchar *s1, const rchar *s2)
{
	return strcmp(s1, s2);
}


rint r_strncmp(const rchar *s1, const rchar *s2, rsize_t n)
{
	return strncmp(s1, s2, (size_t)n);
}


rsize_t r_strlen(const rchar *s)
{
	return strlen(s);
}


rchar *r_strstr(const rchar *haystack, const rchar *needle)
{
	return strstr(haystack, needle);
}
