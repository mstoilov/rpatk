#ifndef _RSTRING_H_
#define _RSTRING_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "rref.h"



typedef struct rstr_s {
	rchar *str;
	ruint size;
} rstr_t;


rint r_strcmp(const rchar *s1, const rchar *s2);
rint r_strncmp(const rchar *s1, const rchar *s2, rsize_t n);
rsize_t r_strlen(const rchar *s);
rchar *r_strstr(const rchar *haystack, const rchar *needle);
rchar *r_strdup(const rchar *s);
rchar *r_strndup(const rchar *s, rsize_t size);
rchar *r_strcpy(rchar *dest, const rchar *src);
rchar *r_strncpy(rchar *dest, const rchar *src, rsize_t n);
rstr_t *r_rstrdup(const rchar *s, ruint size);


typedef struct rstring_s {
	rref_t ref;
	rstr_t s;
} rstring_t;


rstring_t *r_string_create();
rstring_t *r_string_create_from_rstr(const rstr_t *str);
rstring_t *r_string_init(rstring_t *string);
void r_string_destroy(rstring_t *string);
void r_string_cleanup(rstring_t *string);
void r_string_assign(rstring_t *string, const rstr_t *str);
void r_string_cat(rstring_t *string, const rstr_t *str);
rstring_t *r_string_copy(const rstring_t *string);

#ifdef __cplusplus
}
#endif

#endif
