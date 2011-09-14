#include <stdio.h>
#include <string.h>
#include "rpa/rpa.h"


void treewalk_ltor_depthfirst(rarray_t *records, long cur)
{
	rparecord_t *rec;
	long child;

	for (child = rpa_recordtree_firstchild(records, cur, RPA_RECORD_START); child >= 0; child = rpa_recordtree_next(records, child, RPA_RECORD_START)) {
		treewalk_ltor_depthfirst(records, child);
	}

	rec = rpa_record_get(records, cur);
	if (rec->ruleuid == 1) {
		fwrite(rec->input, 1, rec->inputsiz, stdout);
	} else if (rec->ruleuid == 2) {
		/*
		 * mul
		 */
		fprintf(stdout, "*");
	} else if (rec->ruleuid == 3) {
		/*
		 * div
		 */
		fprintf(stdout, "/");
	} else if (rec->ruleuid == 4) {
		/*
		 * sub
		 */
		fprintf(stdout, "-");
	} else if (rec->ruleuid == 5) {
		/*
		 * add
		 */
		fprintf(stdout, "+");
	};
}


int main(int argc, char *argv[])
{
	long i;
	rpadbex_t *dbex;
	rpastat_t *stat;
	rparecord_t *record;
	rarray_t *records = rpa_records_create();

	char expression[] = "9-5*2";
	char bnf[] =
		"#!emitnone\n"
		"#!emitid digit 1\n"
		"#!emitid mul 2\n"
		"#!emitid div 3\n"
		"#!emitid sub 4\n"
		"#!emitid add 5\n"
		"\n"
		"digit		::= [0-9] \n"
		"factor		::= <digit> | '(' <expr> ')' \n"
		"mul		::= <term> '*' <factor> \n"
		"div		::= <term> '/' <factor> \n"
		"term		::= <div> | <mul> | <factor> \n"
		"add		::= <expr> '+' <term> \n"
		"sub		::= <expr> '-' <term> \n"
		"expr		::= <add> | <sub> | <term> \n";

	dbex = rpa_dbex_create();
	rpa_dbex_open(dbex);
	rpa_dbex_load(dbex, bnf, strlen(bnf));
	rpa_dbex_close(dbex);
	rpa_dbex_compile(dbex);
	stat = rpa_stat_create(dbex, RPA_DEFAULT_STACKSIZE);
	rpa_stat_parse(stat, rpa_dbex_last(dbex), RPA_ENCODING_UTF8, expression, expression, expression + sizeof(expression), records);
	for (i = 0; i < rpa_records_length(records); i++) {
		record = rpa_record_get(records, i);
		if (record->type == RPA_RECORD_START)
			fprintf(stdout, "RPA_RECORD_START	(UID: %d)  ", record->ruleuid);
		if (record->type == RPA_RECORD_END)
			fprintf(stdout, "RPA_RECORD_END		(UID: %d)  ", record->ruleuid);
		/*
		 * record->rule points to memory allocated by rpadbex_t,
		 * make sure rpadbex_t object is not destroyed while accessing this pointer.
		 */
		fprintf(stdout, "%s: ", record->rule);
		fwrite(record->input, 1, record->inputsiz, stdout);
		fprintf(stdout, "\n");
	}
	if (rpa_records_length(records) > 0) {
		fprintf(stdout, "\n\n");
		treewalk_ltor_depthfirst(records, 0);
		fprintf(stdout, "\n");
	}
	rpa_records_destroy(records);
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	return 0;
}
