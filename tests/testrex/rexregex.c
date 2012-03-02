#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rex/rexregex.h"

#define BUF_SIZE 0x10000

int main(int argc, char *argv[])
{
	rexregex_t *r;
	char *buf, *q;
	const char *where;
	int l = 0;
	if (argc == 1) {
		fprintf(stderr, "Usage: cat in.file | %s <regexp>\n", argv[0]);
		return 0;
	}

	r = rex_regex_create_s(argv[1]);
	buf = calloc(BUF_SIZE, 1);
	while (fgets(buf, BUF_SIZE - 1, stdin)) {
		++l;
		for (q = buf; *q; ++q);
		if (q > buf)
			*(q-1) = 0;
		if (rex_regex_scan(r, REX_ENCODING_UTF8, buf, q, &where) > 0)
			printf("%d:%s\n", l, buf);
	}
	free(buf);
	rex_regex_destroy(r);
	return 0;
}
