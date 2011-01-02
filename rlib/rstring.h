#ifndef _RSTRING_H_
#define _RSTRING_H_


#include "rtypes.h"
#include "robject.h"

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
rboolean r_stringncmp(const rchar *str, const rchar *s2, rsize_t n);
rsize_t r_strlen(const rchar *s);
rchar *r_strstr(const rchar *haystack, const rchar *needle);
rchar *r_strchr(const rchar *s, rint c);
rchar *r_strdup(const rchar *s);
rchar *r_strndup(const rchar *s, rsize_t size);
rchar *r_strcpy(rchar *dest, const rchar *src);
rchar *r_strncpy(rchar *dest, const rchar *src, rsize_t n);
rchar *r_strcat(rchar *dest, const rchar *src);
rchar *r_strncat(rchar *dest, const rchar *src, ruint n);
rint r_snprintf(rchar *str, ruint size, const rchar *format, ...);
rint r_printf(const rchar *format, ...);

rulong r_strtoul(const rchar *nptr, rchar **endptr, rint base);
rlong r_strtol(const rchar *s, rchar **endptr, rint base);
rdouble r_strtod(const rchar *s, rchar **endptr);

rstr_t *r_rstrdup(const rchar *s, ruint size);


typedef struct rstring_s {
	robject_t obj;
	rstr_t s;
} rstring_t;

#define R_STRING2PTR(__p__) ((rstring_t *)(__p__))->s.str
#define r_string_empty(__p__) (!((__p__) && ((rstring_t *)(__p__))->s.size)) ? 1 : 0
rstring_t *r_string_create();
rstring_t *r_string_create_from_ansistr(const rchar *str);
rstring_t *r_string_create_strsize(const char *str, ruint size);
rstring_t *r_string_create_from_rstr(const rstr_t *str);
robject_t *r_string_init(robject_t *obj, ruint32 type, r_object_cleanupfun cleanup, r_object_copyfun copy);
void r_string_assign(rstring_t *string, const rstr_t *str);
void r_string_cat(rstring_t *string, const rstr_t *str);

/*
 * Virtual methods implementation
 */
void r_string_cleanup(robject_t *obj);
robject_t *r_string_copy(const robject_t *obj);


#ifdef __cplusplus
}
#endif

#endif
