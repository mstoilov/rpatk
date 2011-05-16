#ifndef _RPASTAT_H_
#define _RPASTAT_H_

#include "rtypes.h"
#include "rarray.h"
#include "rvmreg.h"
#include "rpavm.h"
#include "rpadbex.h"
#include "rpacache.h"

#define RPA_ENCODING_UTF8 0
#define RPA_ENCODING_BYTE 1
#define RPA_ENCODING_UTF16LE 2
#define RPA_ENCODING_MASK ((1 << 8) - 1)
#define RPA_ENCODING_ICASE (1 << 8)
#define RPA_ENCODING_ICASE_BYTE (RPA_ENCODING_BYTE | RPA_ENCODING_ICASE)
#define RPA_ENCODING_ICASE_UTF8 (RPA_ENCODING_UTF8 | RPA_ENCODING_ICASE)
#define RPA_ENCODING_ICASE_UTF16LE (RPA_ENCODING_UTF16LE | RPA_ENCODING_ICASE)

#define RPA_DEFAULT_STACKSIZE (256 * 1024)

#ifdef __cplusplus
extern "C" {
#endif



typedef struct rpastat_s rpastat_t;
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


rpastat_t *rpa_stat_create(rpadbex_t *dbex, rulong stacksize);
void rpa_stat_destroy(rpastat_t *stat);
rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end);
void rpa_stat_cachedisable(rpastat_t *stat, ruint disable);
void rpa_stat_cacheinvalidate(rpastat_t *stat);
rint rpa_stat_encodingset(rpastat_t *stat, ruint encoding);

rlong rpa_stat_exec(rpastat_t *stat, rvm_asmins_t *prog, rword off);
rlong rpa_stat_scan(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, const rchar **where);
rlong rpa_stat_match(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end);
rlong rpa_stat_parse(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, rarray_t **records);
rint rpa_stat_abort(rpastat_t *stat);

rint rpa_stat_matchchr(rpastat_t *stat, rssize_t top, rulong wc);
rint rpa_stat_matchspchr(rpastat_t *stat, rssize_t top, rulong wc);
rint rpa_stat_matchrng(rpastat_t *stat, rssize_t top, rulong wc1, rulong wc2);
rlong rpa_stat_shift(rpastat_t *stat, rssize_t top);


#ifdef __cplusplus
}
#endif

#endif
