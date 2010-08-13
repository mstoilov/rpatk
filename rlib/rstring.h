#ifndef _RSTRING_H_
#define _RSTRING_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


rint r_strcmp(const rchar *s1, const rchar *s2);
rint r_strncmp(const rchar *s1, const rchar *s2, rsize_t n);
rsize_t r_strlen(const rchar *s);
rchar *r_strstr(const rchar *haystack, const rchar *needle);

#ifdef __cplusplus
}
#endif

#endif
