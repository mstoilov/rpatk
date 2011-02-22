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
	if (stat->instack)
		r_free(stat->instack);
	r_object_destroy((robject_t*)stat->records);
	rpavm_cpu_destroy(stat->cpu);
	r_free(stat);
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
	if (stat->instacksize < size) {
		stat->instack = r_realloc(stat->instack, (size + 1) * sizeof(rpainput_t));
		stat->instacksize = size + 1;
	}
	stat->ip.input = input;
	stat->ip.serial = 0;
	r_array_setlength(stat->records, 0);
	return 0;
}


rint rpa_stat_parse(rpastat_t *stat, const rchar *input, const rchar *start, const rchar *end)
{
	if (rpa_stat_parse(stat, input, start, end) < 0)
		return -1;


	return 0;
}
