#ifndef _RPASTAT_H_
#define _RPASTAT_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmreg.h"
#include "rpavm.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpacache_s {
	rword disabled;
	rword reclen;
	rword hit;
}rpacache_t;

typedef struct rpastat_s {
	const rchar *input;
	const rchar *start;
	const rchar *end;
	ruint error;
	rarray_t *records;
	rpainput_t *instackbuffer;
	rpainput_t *instack;			/* instack = &instackbuffer[1]; This allows R_TOP = -1, without any additional checks */
	rulong instacksize;
	rulong cursize;
	rpacache_t cache;
	rpainmap_t ip;
	rvmcpu_t *cpu;
} rpastat_t;


rpastat_t *rpa_stat_create(rulong stacksize);
void rpa_stat_destroy(rpastat_t *stat);
rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end);
rint rpa_stat_parse(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end);
void rpa_stat_cachedisable(rpastat_t *stat, ruint disable);
void rpa_stat_cacheinvalidate(rpastat_t *stat);


void rpa_record_dump(rint serial, rparecord_t *rec, rpastat_t *stat);

#ifdef __cplusplus
}
#endif

#endif
