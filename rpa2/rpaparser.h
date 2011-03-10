#ifndef _RPAPARSER_H_
#define _RPAPARSER_H_

#include "rpacompiler.h"
#include "rpastat.h"

#define RPA_PARSER_STACK 8196

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rpa_parser_s {
	rpa_compiler_t *co;
	rpastat_t *stat;
	rulong main;
} rpa_parser_t;


rpa_parser_t *rpa_parser_create();
void rpa_parser_destroy(rpa_parser_t *pa);
rlong rpa_parser_load(rpa_parser_t *pa, const rchar *prods, rsize_t size);
rlong rpa_parser_load_s(rpa_parser_t *pa, const rchar *prods);


#ifdef __cplusplus
}
#endif

#endif
