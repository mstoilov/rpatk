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

#define RPA_STAT_SETERROR_CODE(__d__, __e__) do { (__d__)->err.code = __e__; } while (0)
#define RPA_STAT_SETERRINFO_OFFSET(__d__, __o__) do { (__d__)->err.offset = __o__; (__d__)->err.mask |= RPA_ERRINFO_OFFSET; } while (0)
#define RPA_STAT_SETERRINFO_RULEUID(__d__, __r__) do { (__d__)->err.ruleid = __r__; (__d__)->err.mask |= RPA_ERRINFO_RULEID; } while (0)
#define RPA_STAT_SETERRINFO_NAME(__d__, __n__, __s__) do { \
	(__d__)->err.mask |= RPA_ERRINFO_NAME; \
	r_memset((__d__)->err.name, 0, sizeof((__d__)->err.name)); \
	r_strncpy((__d__)->err.name, __n__, R_MIN(__s__, (sizeof((__d__)->err.name) - 1)));  } while (0)

struct rpastat_s {
	rpadbex_t *dbex;
	const rchar *input;
	const rchar *start;
	const rchar *end;
	rpa_errinfo_t err;
	ruint encoding;
	ruint debug;
	rarray_t *records;
	rpainput_t *instackbuffer;
	rpainput_t *instack;			/* instack = &instackbuffer[1]; This allows R_TOP = -1, without any additional checks */
	rulong instacksize;
	rpacache_t *cache;
	rpainmap_t ip;
	rvmcpu_t *cpu;
};

rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end, rarray_t *records);
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

