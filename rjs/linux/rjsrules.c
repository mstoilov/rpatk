#include "rjs/linux/rjsrules.h"


extern char _binary_____________rjs_ecma262_rpa_start[];
extern char _binary_____________rjs_ecma262_rpa_end[];
extern unsigned long *_binary_____________rjs_ecma262_rpa_size;


const rchar *rjs_rules_get()
{
	const rchar *rules = _binary_____________rjs_ecma262_rpa_start;
	return rules;
}


rsize_t rjs_rules_size()
{
	rsize_t size = _binary_____________rjs_ecma262_rpa_end - _binary_____________rjs_ecma262_rpa_start;
	return size;
}
