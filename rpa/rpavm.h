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

#ifndef _RPAVM_H_
#define _RPAVM_H_

#include "rvm/rvmcpu.h"
#include "rpa/rparecord.h"


#ifdef __cplusplus
extern "C" {
#endif


#define RPA_MATCH_NONE 0
#define RPA_MATCH_MULTIPLE (1 << 0)
#define RPA_MATCH_OPTIONAL (1 << 1)
#define RPA_MATCH_MULTIOPT (RPA_MATCH_MULTIPLE | RPA_MATCH_OPTIONAL)
#define RPA_MATCH_MASK ((1 << 2) - 1)
#define RPA_LOOP_PATH (1<<3)
#define RPA_NONLOOP_PATH (1<<4)
#define RPA_INLINE_REF (1<<5)
#define RPA_LOOP_INDERECTION 1024
#define RPA_RFLAG_EMITRECORD (1 << 0)
#define RPA_RFLAG_ABORTONFAIL (1 << 2)

#define R_REC (TP - 3)
#define R_LOO (TP - 2)
#define R_OTP (TP - 1)
#define R_TOP TP
#define RPAVM_SWI_TABLEID 0

#define RPA_MATCHCHR_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 0))
#define RPA_MATCHCHR_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 1))
#define RPA_MATCHCHR_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 2))
#define RPA_MATCHCHR_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 3))

#define RPA_MATCHRNG_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 4))
#define RPA_MATCHRNG_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 5))
#define RPA_MATCHRNG_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 6))
#define RPA_MATCHRNG_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 7))
#define RPA_MATCHRNG_PEEK	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 8))

#define RPA_MATCHSPCHR_NAN	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 9))
#define RPA_MATCHSPCHR_OPT	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 10))
#define RPA_MATCHSPCHR_MUL	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 11))
#define RPA_MATCHSPCHR_MOP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 12))

#define RPA_SHIFT			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 13))
#define RPA_SETCACHE		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 14))
#define RPA_CHECKCACHE		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 15))

#define RPA_EMITSTART		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 16))
#define RPA_EMITEND			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 17))
#define RPA_EMITTAIL		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 18))
#define RPA_ABORT			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 19))
#define RPA_PRNINFO			RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 20))
#define RPA_MATCHBITMAP		RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 21))
#define RPA_EXITONBITMAP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 22))
#define RPA_VERIFYBITMAP	RVM_OPSWI(RVM_SWI_ID(RPAVM_SWI_TABLEID, 23))



typedef struct rpainput_s {
	const char *input;
	ruint32 wc;
	ruint32 iwc;
	unsigned char eof;
} rpainput_t;


typedef struct rpainmap_s {
	const char *input;
	unsigned long serial;
} rpainmap_t;


typedef struct rpa_ruledata_s {
	ruword size;
	ruword ruleid;
	ruword ruleuid;
	ruword flags;
	ruword namesize;
	ruword name;
} rpa_ruledata_t;


rvmcpu_t *rpavm_cpu_create(unsigned long stacksize);
void rpavm_cpu_destroy(rvmcpu_t * vm);


#ifdef __cplusplus
}
#endif

#endif
