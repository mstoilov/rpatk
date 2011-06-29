#include "rlib/rmem.h"
#include "rlib/rarray.h"
#include "rpa/rpastatpriv.h"
#include "rvm/rvmcodegen.h"
#include "rvm/rvmcpu.h"
#include "rlib/rutf.h"
#include "rlib/rcharconv.h"


rpastat_t *rpa_stat_create(rpadbex_t *dbex, rulong stacksize)
{
	rpastat_t *stat = (rpastat_t *) r_zmalloc(sizeof(*stat));
	if (stacksize == 0)
		stacksize = RPA_DEFAULT_STACKSIZE;
	stat->cpu = rpavm_cpu_create(stacksize);
	stat->cache = rpa_cache_create();
	if (!stat->cpu) {
		r_free(stat);
		return NULL;
	}
	stat->dbex = dbex;
	stat->cpu->userdata1 = stat;
	return stat;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat) {
		if (stat->instackbuffer)
			r_free(stat->instackbuffer);
		rpavm_cpu_destroy(stat->cpu);
		rpa_cache_destroy(stat->cache);
		r_free(stat);
	}
}


void rpa_stat_cachedisable(rpastat_t *stat, ruinteger disable)
{
	rpa_cache_disable(stat->cache, disable);
}


rinteger rpa_stat_init(rpastat_t *stat, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end, rarray_t *records)
{
	rulong size;

	if (!stat) {
		return -1;
	}
	if (start > end) {
		return -1;
	}
	if (input < start || input > end) {
		return -1;
	}
	r_memset(&stat->err, 0, sizeof(stat->err));
	size = end - start;
	stat->encoding = encoding;
	stat->start = start;
	stat->end = end;
	stat->input = input;
	stat->cache->hit = 0;
	stat->records = records;
	if (stat->instacksize == 0 || stat->instacksize < size) {
		stat->instackbuffer = r_realloc(stat->instackbuffer, (size + 2) * sizeof(rpainput_t));
		stat->instacksize = size;
		stat->instack = &stat->instackbuffer[1];
		r_memset(stat->instackbuffer, 0, sizeof(rpainput_t) * 2);

	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	RVM_CPUREG_SETL(stat->cpu, R_REC, 0);
	RVM_CPUREG_SETU(stat->cpu, SP, 0);
	RVM_CPUREG_SETU(stat->cpu, FP, 0);
	RVM_CPUREG_SETU(stat->cpu, R_LOO, 0);
	RVM_CPUREG_SETU(stat->cpu, R_TOP, -1);
	if (stat->records) {
		RVM_CPUREG_SETL(stat->cpu, R_REC, (rlong)(r_array_length(stat->records) - 1));
	}
	return 0;
}


void rpa_stat_cacheinvalidate(rpastat_t *stat)
{
	rpa_cache_invalidate(stat->cache);
}


rlong rpa_stat_exec(rpastat_t *stat, rvm_asmins_t *prog, rword off, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end, rarray_t *records)
{
	rlong ret;

	if (!stat) {
		return -1;
	}
	rpa_stat_cacheinvalidate(stat);
	rpa_stat_init(stat, encoding, input, start, end, records);

	if (stat->debug) {
		ret = rvm_cpu_exec_debug(stat->cpu, prog, off);
	} else {
		ret = rvm_cpu_exec(stat->cpu, prog, off);
	}
	if (ret < 0) {
		if (!stat->cpu->error) {
			if (stat->cpu->error) {
				RPA_STAT_SETERROR_CODE(stat, stat->cpu->error);
			} else {
				/*
				 * We should never get to here. Error have to be more
				 * specific and set at the places they are detected.
				 */
				RPA_STAT_SETERROR_CODE(stat, RPA_E_EXECUTION);
			}
		}
		return -1;
	}
	ret = (rlong)RVM_CPUREG_GETL(stat->cpu, R0);
	if (ret < 0) {
		return 0;
	}
	return ret;
}


static rlong rpa_stat_exec_rid(rpastat_t *stat, rparule_t rid, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end, rarray_t *records)
{
	rlong topsiz = 0;
	rpainput_t *ptp;
	rlong offset;
	rvm_asmins_t *exec;


	exec = rpa_dbex_executable(stat->dbex);
	if (!exec) {
		RPA_STAT_SETERROR_CODE(stat, RPA_E_EXECUTION);
		return -1;
	}
	offset = rpa_dbex_executableoffset(stat->dbex, rid);
	if (offset < 0) {
		RPA_STAT_SETERROR_CODE(stat, RPA_E_EXECUTION);
		return -1;
	}
	if ((topsiz = rpa_stat_exec(stat, exec, offset, encoding, input, start, end, records)) < 0) {
		return -1;
	}
	if (topsiz <= 0)
		return 0;
	ptp = &stat->instack[topsiz];
	return (ptp->input - input);
}


rlong rpa_stat_scan(rpastat_t *stat, rparule_t rid, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end, const rchar **where)
{
	rlong ret;

	while (input < end) {
		ret = rpa_stat_exec_rid(stat, rid, encoding, input, start, end, NULL);
		if (ret < 0)
			return -1;
		if (ret > 0) {
			*where = input;
			return ret;
		}
		input += 1;
	}
	return ret;
}


rlong rpa_stat_match(rpastat_t *stat, rparule_t rid, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end)
{
	return rpa_stat_exec_rid(stat, rid, encoding, input, start, end, NULL);
}


rlong rpa_stat_parse(rpastat_t *stat, rparule_t rid, ruinteger encoding, const rchar *input, const rchar *start, const rchar *end, rarray_t *records)
{
	return rpa_stat_exec_rid(stat, rid, encoding, input, start, end, records);
}


rinteger rpa_stat_abort(rpastat_t *stat)
{
	if (!stat) {
		return -1;
	}
	RPA_STAT_SETERROR_CODE(stat, RPA_E_USERABORT);
	rvm_cpu_abort(stat->cpu);
	return 0;
}


rinteger rpa_stat_matchchr(rpastat_t *stat, rssize_t top, rulong wc)
{
	rinteger ret = 0;
	rpainput_t *in = &stat->instack[top];

	if (in->eof)
		return 0;
	if (stat->encoding & RPA_ENCODING_ICASE) {
		ret = (in->wc == wc || in->iwc == wc) ? 1 : 0;
	} else {
		ret = (in->wc == wc) ? 1 : 0;
	}
	return ret;
}


rinteger rpa_stat_matchspchr(rpastat_t *stat, rssize_t top, rulong wc)
{
	rinteger ret = 0;
	rpainput_t *in = &stat->instack[top];

	if (in->eof)
		return 0;

	switch (wc) {
		case '.':
			return 1;
			break;
		case 't':
			wc = '\t';
			break;
		case 'r':
			wc = '\r';
			break;
		case 'n':
			wc = '\n';
			break;
		default:
			break;
	};

	ret = (in->wc == wc) ? 1 : 0;
	return ret;
}


rinteger rpa_stat_matchrng(rpastat_t *stat, rssize_t top, rulong wc1, rulong wc2)
{
	rinteger ret = 0;
	rpainput_t *in = &stat->instack[top];

	if (in->eof)
		return 0;
	if (stat->encoding & RPA_ENCODING_ICASE) {
		ret = ((in->wc >= wc1 && in->wc <= wc2) || (in->iwc >= wc1 && in->iwc <= wc2)) ? 1 : 0;
	} else {
		ret = ((in->wc >= wc1 && in->wc <= wc2)) ? 1 : 0;
	}
	return ret;
}


static rinteger rpa_stat_utf8_getchar(ruint32 *pwc, rpastat_t *stat, const rchar *input)
{
	return r_utf8_mbtowc(pwc, (const ruchar*)input, (const ruchar*)stat->end);
}


static rinteger rpa_stat_byte_getchar(ruint32 *pwc, rpastat_t *stat, const rchar *input)
{
	if (input >= stat->end) {
		*pwc = (unsigned int)0;
		return 0;
	}
	*pwc = *((const ruchar*)input);
	return 1;
}


static int rpa_stat_utf16_getchar(ruint32 *pwc, rpastat_t *stat, const rchar *input)
{
	return r_utf16_mbtowc(pwc, (const ruchar*)input, (const ruchar*)stat->end);
}


rlong rpa_stat_shift(rpastat_t *stat, rssize_t top)
{
	rpainput_t * ptp = &stat->instack[top];

	if (ptp->eof)
		return -1;
	ptp++;
	top++;
	if (top >= (rlong)stat->ip.serial) {
		rinteger inc = 0;
		ptp->input = stat->ip.input;
		if (ptp->input < stat->end) {
			switch (stat->encoding & RPA_ENCODING_MASK) {
			default:
			case RPA_ENCODING_UTF8:
				inc = rpa_stat_utf8_getchar(&ptp->wc, stat, (const rchar*)stat->ip.input);
				break;
			case RPA_ENCODING_UTF16LE:
				inc = rpa_stat_utf16_getchar(&ptp->wc, stat, (const rchar*)stat->ip.input);
				break;
			case RPA_ENCODING_BYTE:
				inc = rpa_stat_byte_getchar(&ptp->wc, stat, (const rchar*)stat->ip.input);
				break;
			};
			if (stat->encoding & RPA_ENCODING_ICASE)
				ptp->iwc = r_charicase(ptp->wc);
			stat->ip.input += inc;
			stat->ip.serial += 1;
			ptp->eof = 0;
		} else {
			ptp->wc = (ruint32)-1;
			ptp->eof = 1;
		}
	}

	return top;
}


rlong rpa_stat_lasterror(rpastat_t *stat)
{
	if (!stat)
		return -1;
	return stat->err.code;
}


rlong rpa_stat_lasterrorinfo(rpastat_t *stat, rpa_errinfo_t *errinfo)
{
	if (!stat || !errinfo)
		return -1;
	r_memcpy(errinfo, &stat->err, sizeof(rpa_errinfo_t));
	return 0;
}
