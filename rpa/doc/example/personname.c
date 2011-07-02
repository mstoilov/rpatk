#include <stdio.h>
#include <string.h>
#include "rpa/rpa.h"

/*
 * This example shows the sequence of RPA APIs to parse an input. No error checking is done,
 * to keep the example simple and make the API call sequence easy to follow.
 *
 * To compile and run this sample on Linux:
 * 1. Install Rpa/Tk:
 * # cd rpatk/build/linux/x86_64
 * # make
 * # make install (as superuser)
 *
 * 2. Build this example:
 * # gcc -o personname personname.c -I/usr/include/rpatk -lrpa -lrvm -lrlib -lm
 *
 * 3. Run it, and you should get:
 * # ./personname
 * RPA_RECORD_START   (ID: 1)  name: John M. Smith
 * RPA_RECORD_START   (ID: 2)  first: John
 * RPA_RECORD_END     (ID: 2)  first: John
 * RPA_RECORD_START   (ID: 0)  middle: M.
 * RPA_RECORD_END     (ID: 0)  middle: M.
 * RPA_RECORD_START   (ID: 3)  last: Smith
 * RPA_RECORD_END     (ID: 3)  last: Smith
 * RPA_RECORD_END     (ID: 1)  name: John M. Smith
 */
int main(int argc, char *argv[])
{
	rlong i;
	rpadbex_t *dbex;
	rpastat_t *stat;
	rparecord_t *record;
	rarray_t *records = rpa_records_create();

	char name[] = "John M. Smith";
	char bnf[] = 
		"#!emitid name 1\n"
		"#!emitid first 2\n"
		"#!emitid last 3\n"
		"\n"
		"first  ::= [A-Za-z]+\n"
		"middle ::= [A-Za-z]+ '.'?\n"
		"last   ::= [A-Za-z]+\n"
		"name   ::= <first> ' ' <middle> ' ' <last>\n";

	dbex = rpa_dbex_create();
	rpa_dbex_open(dbex);
	rpa_dbex_load(dbex, bnf, strlen(bnf));
	rpa_dbex_close(dbex);
	rpa_dbex_compile(dbex);
	stat = rpa_stat_create(dbex, RPA_DEFAULT_STACKSIZE);
	rpa_stat_parse(stat, rpa_dbex_last(dbex), RPA_ENCODING_UTF8, name, name, name+sizeof(name), records);
	for (i = 0; i < rpa_records_length(records); i++) {
		record = rpa_records_slot(records, i);
		if (record->type == RPA_RECORD_START)
			fprintf(stdout, "RPA_RECORD_START   (UID: %d)  ", record->ruleuid);
		if (record->type == RPA_RECORD_END)
			fprintf(stdout, "RPA_RECORD_END     (UID: %d)  ", record->ruleuid);
		fprintf(stdout, "%s: ", rpa_dbex_name(dbex, record->ruleid));
		fwrite(&name[record->inputoff], 1, record->inputsiz, stdout);
		fprintf(stdout, "\n");
	}
	rpa_records_destroy(records);
	rpa_stat_destroy(stat);
	rpa_dbex_destroy(dbex);
	return 0;
}
