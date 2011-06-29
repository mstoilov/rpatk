#include <stdio.h>
#include "rpa/rpa/rpa.h"


int main(int argc, char *argv[])
{
	rpadbex_t *dbex;
	rpastat_t *stat;
	char name[] = "John M. Smith";
	char bnf[] = 
		"first  ::= [A-Za-z]+\n"
		"middle ::= [A-Za-z]+ '.'?\n"
		"last   ::= [A-Za-z]+\n"
		"name   ::= <first> ' ' <middle> ' ' <last>\n";

	dbex = rpa_dbex_create();
	rpa_dbex_load(dbex, bnf);
	rpa_dbex_close(dbex);
	rpa_dbex_compile(dbex);


	return 0;
}
