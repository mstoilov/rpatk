#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rex/rexregex.h"

/*
 * To build:
 * gcc -o rexregex rexregex.c -I/usr/include/rpatk -lrex -lrlib
 */

#define BUF_SIZE 0x10000

int main(int argc, char *argv[])
{
	rexregex_t *regex;
	char *buf, *end;
	const char *where;
	int line = 0;

	if (argc < 2) {
		fprintf(stderr, "Usage:\n cat <file> | %s <regexp>\n", argv[0]);
		return 0;
	}
	regex = rex_regex_create_s(argv[1]);
	buf = calloc(BUF_SIZE, 1);
	while (fgets(buf, BUF_SIZE - 1, stdin)) {
		++line;
		for (end = buf; *end; ++end);
		if (end > buf)
			*(end - 1) = '\0';
		if (rex_regex_scan(regex, REX_ENCODING_BYTE, buf, end, &where) > 0)
			printf("%d: %s\n", line, buf);
	}
	free(buf);
	rex_regex_destroy(regex);
	return 0;
}
