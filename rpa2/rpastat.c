#include "rmem.h"
#include "rarray.h"
#include "rpastat.h"
#include "rpaerror.h"
#include "rvmcodegen.h"
#include "rvmcpu.h"
#include "rutf.h"
#include "rcharconv.h"


#define RPA_STAT_SETERROR_CODE(__s__, __e__) do { (__s__)->error = __e__; } while (0)


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
	stat->records = r_array_create(sizeof(rparecord_t));
	stat->emitstack = r_array_create(sizeof(rlong));
	stat->orphans = r_array_create(sizeof(rlong));
	stat->cpu->userdata1 = stat;

	return stat;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat) {
		if (stat->instack)
			r_free(stat->instackbuffer);
		r_array_destroy(stat->records);
		r_array_destroy(stat->emitstack);
		r_array_destroy(stat->orphans);
		rpavm_cpu_destroy(stat->cpu);
		rpa_cache_destroy(stat->cache);
		r_free(stat);
	}
}


void rpa_stat_cachedisable(rpastat_t *stat, ruint disable)
{
	rpa_cache_disable(stat->cache, disable);
}


rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
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
	size = end - start;
	stat->start = start;
	stat->end = end;
	stat->input = input;
	stat->error = 0;
	r_array_setlength(stat->orphans, 0);
	r_array_setlength(stat->emitstack, 0);
	stat->cache->hit = 0;
	if (stat->instacksize < size) {
		stat->instackbuffer = r_realloc(stat->instackbuffer, (size + 2) * sizeof(rpainput_t));
		stat->instacksize = size + 1;
		stat->instack = &stat->instackbuffer[1];
		r_memset(stat->instackbuffer, 0, sizeof(rpainput_t) * 2);
	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	return 0;
}


void rpa_stat_cacheinvalidate(rpastat_t *stat)
{
	rpa_cache_invalidate(stat->cache);
}


rint rpa_stat_encodingset(rpastat_t *stat, ruint encoding)
{
	if (!stat) {
		return -1;
	}

	stat->encoding = encoding;
	return 0;
}


static rparecord_t *rpa_stat_nextrecord(rpastat_t *stat, rparecord_t *cur)
{
	if (r_array_empty(stat->records))
		return NULL;
	if (cur == NULL) {
		return (rparecord_t *)r_array_slot(stat->records, 0);
	}
	if (cur >= (rparecord_t *)r_array_lastslot(stat->records) || cur < (rparecord_t *)r_array_slot(stat->records, 0))
		return NULL;
	cur = (rparecord_t *)r_array_slot(stat->records, cur->next);
	if (cur->type == RPA_RECORD_TAIL)
		return NULL;
	return cur;
}


static void rpa_stat_fixrecords(rpastat_t *stat)
{
	rarray_t *records;
	rparecord_t *cur = (rparecord_t *)r_array_slot(stat->records, 0);

	cur = rpa_stat_nextrecord(stat, cur);
	if (!cur) {
		/*
		 * There are no records
		 */
		r_array_setlength(stat->records, 0);
		return;
	}
	records = (rarray_t *)r_array_create(sizeof(rparecord_t));
	while (cur) {
		r_array_add(records, cur);
		cur = rpa_stat_nextrecord(stat, cur);
	}
	r_array_destroy(stat->records);
	stat->records = records;
}



rlong rpa_stat_exec(rpastat_t *stat, rvm_asmins_t *prog, rword off)
{
	rlong ret;

	if (!stat) {
		return -1;
	}
	rpa_stat_cacheinvalidate(stat);
	r_array_setlength(stat->records, 0);
	RVM_CPUREG_SETU(stat->cpu, SP, 0);
	RVM_CPUREG_SETU(stat->cpu, FP, 0);
	RVM_CPUREG_SETL(stat->cpu, R_REC, -1);
	RVM_CPUREG_SETU(stat->cpu, R_LOO, 0);
	RVM_CPUREG_SETU(stat->cpu, R_TOP, -1);
	if (stat->debug) {
		ret = rvm_cpu_exec_debug(stat->cpu, prog, off);
	} else {
		ret = rvm_cpu_exec(stat->cpu, prog, off);
	}
	if (ret < 0) {
		if (stat->cpu->error == RVM_E_USERABORT)
			RPA_STAT_SETERROR_CODE(stat, RPA_E_USERABORT);
		else
			RPA_STAT_SETERROR_CODE(stat, stat->cpu->error);
		r_array_setlength(stat->records, 0);
		return -1;
	}
	rpa_stat_fixrecords(stat);
	ret = (rlong)RVM_CPUREG_GETL(stat->cpu, R0);
	if (ret < 0) {
		r_array_setlength(stat->records, 0);
		return 0;
	}
	return ret;
}


static rlong rpa_stat_exec_noinit(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end)
{
	rlong topsiz = 0;
	rpainput_t *ptp;

	rpa_stat_init(stat, input, start, end);
	if ((topsiz = rpa_stat_exec(stat, rvm_dbex_getexecutable(stat->dbex), rvm_dbex_executableoffset(stat->dbex, rid))) < 0) {
		return -1;
	}
	if (topsiz <= 0)
		return 0;
	ptp = &stat->instack[topsiz];
	return (ptp->input - input);
}


rlong rpa_stat_scan(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, const rchar **where)
{
	rlong ret;

	if (!stat) {
		return -1;
	}
	while (input < end) {
		ret = rpa_stat_exec_noinit(stat, rid, input, start, end);
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


rlong rpa_stat_match(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end)
{
	if (!stat) {
		return -1;
	}

	return rpa_stat_exec_noinit(stat, rid, input, start, end);
}


rlong rpa_stat_parse(rpastat_t *stat, rparule_t rid, const rchar *input, const rchar *start, const rchar *end, rarray_t **records)
{
	rint res = 0;

	if (!stat) {
		return -1;
	}

	res = rpa_stat_exec_noinit(stat, rid, input, start, end);
	if (res > 0 && !r_array_empty(stat->records) && records) {
		*records = stat->records;
		stat->records = r_array_create(sizeof(rparecord_t));
	}
	return res;
}


rint rpa_stat_abort(rpastat_t *stat)
{
	if (!stat) {
		return -1;
	}

	rvm_cpu_abort(stat->cpu);
	return 0;
}


rint rpa_stat_matchchr(rpastat_t *stat, rssize_t top, rulong wc)
{
	rint ret = 0;
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


rint rpa_stat_matchspchr(rpastat_t *stat, rssize_t top, rulong wc)
{
	rint ret = 0;
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


rint rpa_stat_matchrng(rpastat_t *stat, rssize_t top, rulong wc1, rulong wc2)
{
	rint ret = 0;
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


static rint rpa_stat_utf8_getchar(ruint32 *pwc, rpastat_t *stat, const rchar *input)
{
	return r_utf8_mbtowc(pwc, (const ruchar*)input, (const ruchar*)stat->end);
}


static rint rpa_stat_byte_getchar(ruint32 *pwc, rpastat_t *stat, const rchar *input)
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
		rint inc = 0;
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
