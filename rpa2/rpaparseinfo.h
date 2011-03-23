#ifndef _RPAPARSERINFO_H_
#define _RPAPARSERINFO_H_

#include "rharray.h"
#include "rpacompiler.h"
#include "rpastat.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_ruleinfo_s {
	rlong startrec;
	rlong sizerecs;
	rlong startref;
	rlong sizerefs;
} rpa_ruleinfo_t;


typedef struct rpa_parseinfo_s {
	rarray_t *records;
	rharray_t *rules;
	rarray_t *recstack;
} rpa_parseinfo_t;


rpa_parseinfo_t *rpa_parseinfo_create(rpastat_t *stat);
void rpa_parseinfo_destroy(rpa_parseinfo_t *pi);
void rpa_parseinfo_dump(rpa_parseinfo_t *pi);
void rpa_parseinfo_dump_records(rpa_parseinfo_t *pi);
rint rpa_parseinfo_dump_ruletree(rpa_parseinfo_t *pi, const char *rulename);

#ifdef __cplusplus
}
#endif

#endif
