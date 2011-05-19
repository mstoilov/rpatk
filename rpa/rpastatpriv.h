#ifndef _RPASTATPRIV_H_
#define _RPASTATPRIV_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmreg.h"
#include "rpavm.h"
#include "rpadbex.h"
#include "rpacache.h"
#include "rpastat.h"


#ifdef __cplusplus
extern "C" {
#endif


struct rpastat_s {
	rpadbex_t *dbex;
	const rchar *input;
	const rchar *start;
	const rchar *end;
	ruint error;
	ruint encoding;
	ruint debug;
	rarray_t *records;
	rarray_t *emitstack;
	rarray_t *orphans;
	rpainput_t *instackbuffer;
	rpainput_t *instack;			/* instack = &instackbuffer[1]; This allows R_TOP = -1, without any additional checks */
	rulong instacksize;
	rpacache_t *cache;
	rpainmap_t ip;
	rvmcpu_t *cpu;
};

rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end);
void rpa_stat_cachedisable(rpastat_t *stat, ruint disable);
void rpa_stat_cacheinvalidate(rpastat_t *stat);
rint rpa_stat_matchchr(rpastat_t *stat, rssize_t top, rulong wc);
rint rpa_stat_matchspchr(rpastat_t *stat, rssize_t top, rulong wc);
rint rpa_stat_matchrng(rpastat_t *stat, rssize_t top, rulong wc1, rulong wc2);
rlong rpa_stat_shift(rpastat_t *stat, rssize_t top);


#ifdef __cplusplus
}
#endif

#endif

