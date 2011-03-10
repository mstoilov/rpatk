#include "rmem.h"
#include "rarray.h"
#include "rpastat.h"


rpastat_t *rpa_stat_create(rulong stacksize)
{
	rpastat_t *stat = (rpastat_t *) r_zmalloc(sizeof(*stat));
	stat->cpu = rpavm_cpu_create(stacksize);
	if (!stat->cpu) {
		r_free(stat);
		return NULL;
	}
	stat->records = r_array_create(sizeof(rparecord_t));
	stat->cpu->userdata1 = stat;
	return stat;
}


void rpa_stat_destroy(rpastat_t *stat)
{
	if (stat) {
		if (stat->instack)
			r_free(stat->instackbuffer);
		r_object_destroy((robject_t*)stat->records);
		rpavm_cpu_destroy(stat->cpu);
		r_free(stat);
	}
}


void rpa_stat_cachedisable(rpastat_t *stat, ruint disable)
{
	stat->cache.disabled = disable;
}


rint rpa_stat_init(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
{
	rulong size;

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
	stat->cursize = 0;
	stat->cache.reclen = 0;
	stat->cache.hit = 0;
	if (stat->instacksize < size) {
		stat->instackbuffer = r_realloc(stat->instackbuffer, (size + 2) * sizeof(rpainput_t));
		stat->instacksize = size + 1;
		stat->instack = &stat->instackbuffer[1];
		r_memset(stat->instackbuffer, 0, sizeof(rpainput_t) * 2);
	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	r_array_setlength(stat->records, 0);
	return 0;
}


void rpa_stat_cacheinvalidate(rpastat_t *stat)
{
	stat->cache.reclen = 0;
}


rint rpa_stat_parse(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
{

	return 0;
}


void rpa_record_dump(rint serial, rparecord_t *rec, rpastat_t *stat)
{
	rchar buf[240];
	rint bufsize = sizeof(buf) - 1;
	rint n = 0, size;

	r_memset(buf, 0, bufsize);

	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%3d ( %7ld ): ", serial, rec->ruleid);
	if (rec->type & RPA_RECORD_START)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "START ");
	if (rec->type & RPA_RECORD_MATCH)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "MATCH ");
	if (rec->type & RPA_RECORD_END)
		n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "END ");
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, "%s(%d) ", rec->rule, rec->type);

	r_memset(buf + n, ' ', bufsize - n);
	n = 55;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : %5d, %3d", rec->top, rec->size);


	r_memset(buf + n, ' ', bufsize - n);
	n = 75;
	n += r_snprintf(buf + n, n < bufsize ? bufsize - n : 0, " : ");
	size = rec->inputsiz;
	if (size >= bufsize - n - 1)
		size = bufsize - n - 1;
	if (rec->type & RPA_RECORD_END) {
		r_strncpy(buf + n, rec->input, rec->inputsiz);
		n += size;
		buf[n] = '\0';
	}

	r_printf("%s\n", buf);
}
