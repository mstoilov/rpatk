#include "rjs/rjsrules.h"


extern char _binary_____________rjs_ecma262_rpa_start[];
extern char _binary_____________rjs_ecma262_rpa_end[];
extern unsigned long *_binary_____________rjs_ecma262_rpa_size;


const char *rjs_rules_get()
{
	const char *rules = _binary_____________rjs_ecma262_rpa_start;
	return rules;
}


unsigned long rjs_rules_size()
{
	unsigned long size = _binary_____________rjs_ecma262_rpa_end - _binary_____________rjs_ecma262_rpa_start;
	return size;
}
