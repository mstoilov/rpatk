#include "rjs/rjsrules.h"


extern char _binary____ecma262_rpa_start[];
extern char _binary____ecma262_rpa_end[];
extern unsigned long *_binary____ecma262_rpa_size;


const char *rjs_rules_get()
{
	const char *rules = _binary____ecma262_rpa_start;
	return rules;
}


unsigned long rjs_rules_size()
{
	unsigned long size = _binary____ecma262_rpa_end - _binary____ecma262_rpa_start;
	return size;
}
