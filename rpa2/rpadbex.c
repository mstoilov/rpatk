#include "rpadbex.h"
#include "rpaparser.h"
#include "rpaparseinfo.h"
#include "rmem.h"

struct rpadbex_s {
	rpa_parser_t *pa;
	rpa_parseinfo_t *pi;
};


static rparecord_t *rpa_dbex_record(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex || !dbex->pi)
		return NULL;
	if (rid < 0 || rid >= r_array_length(dbex->pi->records))
		return NULL;
	prec = (rparecord_t *)r_array_slot(dbex->pi->records, rid);
	return prec;
}


rpadbex_t *rpa_dbex_create(void)
{
	rpadbex_t *dbex = (rpadbex_t *) r_zmalloc(sizeof(*dbex));

	dbex->pa = rpa_parser_create();

	return dbex;
}


void rpa_dbex_destroy(rpadbex_t *dbex)
{
	if (dbex) {
		rpa_parser_destroy(dbex->pa);
		rpa_parseinfo_destroy(dbex->pi);
		r_free(dbex);
	}
}


rint rpa_dbex_open(rpadbex_t *dbex)
{
	if (!dbex)
		return -1;
	if (dbex->pi) {
		rpa_parseinfo_destroy(dbex->pi);
		dbex->pi = NULL;
	}
	return 0;
}


void rpa_dbex_close(rpadbex_t *dbex)
{
	if (!dbex)
		return;
	if (dbex->pi) {
		rpa_parseinfo_destroy(dbex->pi);
		dbex->pi = NULL;
	}
	dbex->pi = rpa_parseinfo_create(dbex->pa->stat);
}


rlong rpa_dbex_load(rpadbex_t *dbex, const rchar *rules, rsize_t size)
{
	if (!dbex)
		return -1;
	if (dbex->pi) {
		/*
		 * Dbex is not open
		 */
		return -1;
	}
	return rpa_parser_load(dbex->pa, rules, size);
}


rlong rpa_dbex_load_s(rpadbex_t *dbex, const rchar *rules)
{
	return rpa_dbex_load(dbex, rules, r_strlen(rules));
}


rint rpa_dbex_dumptree(rpadbex_t *dbex, const rchar *name)
{
	if (!dbex || !dbex->pi)
		return -1;
	return rpa_parseinfo_dump_ruletree(dbex->pi, name);
}


rint rpa_dbex_dumprules(rpadbex_t *dbex)
{
	rint ret = 0;
	rpa_parseinfo_t *pi;
	rparule_t rid;
	rchar buffer[512];

	if (!dbex || !dbex->pa)
		return -1;
	pi = rpa_parseinfo_create(dbex->pa->stat);
	for (rid = rpa_dbex_rule_first(dbex); rid >= 0; rid = rpa_dbex_rule_next(dbex, rid)) {
		if (rpa_dbex_rule_copy(dbex, rid, buffer, sizeof(buffer)) >= 0)
			r_printf("   %s\n", buffer);
	}
	rpa_parseinfo_destroy(pi);
	return ret;
}


rlong rpa_dbex_rule_copy(rpadbex_t *dbex, rparule_t rid, rchar *buf, rsize_t bufsize)
{
	rparecord_t *prec;

	if (!dbex || !dbex->pi)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	if (bufsize <= prec->inputsiz)
		return -1;
	r_memset(buf, 0, bufsize);
	r_strncpy(buf, prec->input, prec->inputsiz);
	return prec->inputsiz;
}


rlong rpa_dbex_rule_size(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex || !dbex->pi)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	return prec->inputsiz;
}


rparule_t rpa_dbex_rule_first(rpadbex_t *dbex)
{
	rparecord_t *prec;
	rparule_t rid = 0;

	if (!dbex || !dbex->pi)
		return -1;
again:
	rid = rpa_recordtree_get(dbex->pi->records, rid, RPA_RECORD_END);
	if (rid < 0)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	if (prec->userid != RPA_PRODUCTION_NAMEDRULE || !(rpa_record_getusertype(dbex->pi->records, rid) & RPA_RULE_USED)) {
		rid = rpa_recordtree_next(dbex->pi->records, rid, RPA_RECORD_END);
		goto again;
	}
	return rid;
}


rparule_t rpa_dbex_rule_next(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex || !dbex->pi)
		return -1;
again:
	rid = rpa_recordtree_next(dbex->pi->records, rid, RPA_RECORD_END);
	if (rid < 0)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	if (prec->userid != RPA_PRODUCTION_NAMEDRULE || !(rpa_record_getusertype(dbex->pi->records, rid) & RPA_RULE_USED))
		goto again;
	return rid;
}


rparule_t rpa_dbex_rule_prev(rpadbex_t *dbex, rparule_t rid)
{
	rparecord_t *prec;

	if (!dbex || !dbex->pi)
		return -1;
again:
	rid = rpa_recordtree_prev(dbex->pi->records, rid, RPA_RECORD_END);
	if (rid < 0)
		return -1;
	if ((prec = rpa_dbex_record(dbex, rid)) == NULL)
		return -1;
	if (prec->userid != RPA_PRODUCTION_NAMEDRULE || !(rpa_record_getusertype(dbex->pi->records, rid) & RPA_RULE_USED))
		goto again;
	return rid;

}


ruint rpa_dbex_get_error(rpadbex_t *dbex);
rint rpa_dbex_compile(rpadbex_t *dbex);

const rchar *rpa_dbex_version();
