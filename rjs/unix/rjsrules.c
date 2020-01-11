#include "rjs/rjsrules.h"

#ifdef SIMPLE_EBNF
#define ECMA262_NAME(name) _binary_____________rjs_ecma262_ebnf_##name
#else
#define ECMA262_NAME(name) _binary_____________rjs_ecma262_rpa_##name
#endif

extern char ECMA262_NAME(start)[];
extern char ECMA262_NAME(end)[];
extern unsigned long *ECMA262_NAME(size);


const char *rjs_rules_get()
{
	const char *rules = ECMA262_NAME(start);
	return rules;
}


unsigned long rjs_rules_size()
{
	unsigned long size = ECMA262_NAME(end) - ECMA262_NAME(start);
	return size;
}
