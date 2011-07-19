/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

#ifndef _RPASTATPRIV_H_
#define _RPASTATPRIV_H_

#include "rtypes.h"
#include "rlib/rarray.h"
#include "rvm/rvmreg.h"
#include "rpa/rpavm.h"
#include "rpa/rpadbex.h"
#include "rpa/rpacache.h"
#include "rpa/rpastat.h"
#include "rpa/rpabitmap.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_STAT_SETERROR_CODE(__d__, __e__) do { (__d__)->err.code = __e__; (__d__)->err.mask |= RPA_ERRINFO_CODE;} while (0)
#define RPA_STAT_SETERRINFO_OFFSET(__d__, __o__) do { (__d__)->err.offset = __o__; (__d__)->err.mask |= RPA_ERRINFO_OFFSET; } while (0)
#define RPA_STAT_SETERRINFO_RULEUID(__d__, __r__) do { (__d__)->err.ruleuid = __r__; (__d__)->err.mask |= RPA_ERRINFO_RULEUID; } while (0)
#define RPA_STAT_SETERRINFO_NAME(__d__, __n__, __s__) do { \
	(__d__)->err.mask |= RPA_ERRINFO_NAME; \
	r_memset((__d__)->err.name, 0, sizeof((__d__)->err.name)); \
	r_strncpy((__d__)->err.name, __n__, R_MIN(__s__, (sizeof((__d__)->err.name) - 1)));  } while (0)

struct rpastat_s {
	rpadbex_t *dbex;
	const char *input;
	const char *start;
	const char *end;
	rpa_errinfo_t err;
	unsigned int encoding;
	unsigned int debug;
	rarray_t *records;
	rpainput_t *instackbuffer;
	rpainput_t *instack;			/* instack = &instackbuffer[1]; This allows R_TOP = -1, without any additional checks */
	unsigned long instacksize;
	rpacache_t *cache;
	rpainmap_t ip;
	rvmcpu_t *cpu;
};

int rpa_stat_init(rpastat_t *stat, unsigned int encoding, const char *input, const char *start, const char *end, rarray_t *records);
void rpa_stat_cachedisable(rpastat_t *stat, unsigned int disable);
void rpa_stat_cacheinvalidate(rpastat_t *stat);
rboolean rpa_stat_matchbitmap(rpastat_t *stat, rssize_t top, rpabitmap_t bitmap);
int rpa_stat_matchchr(rpastat_t *stat, rssize_t top, unsigned long wc);
int rpa_stat_matchspchr(rpastat_t *stat, rssize_t top, unsigned long wc);
int rpa_stat_matchrng(rpastat_t *stat, rssize_t top, unsigned long wc1, unsigned long wc2);
long rpa_stat_shift(rpastat_t *stat, rssize_t top);
unsigned long rpa_special_char(unsigned long special);

#ifdef __cplusplus
}
#endif

#endif

