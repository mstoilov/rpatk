#ifndef _RPAPARSER_H_
#define _RPAPARSER_H_

#include "rpacompiler.h"
#include "rpastat.h"

#define RPA_PARSER_STACK 8196

#ifdef __cplusplus
extern "C" {
#endif

enum {
	RPA_PRODUCTION_NONE = 0,
	RPA_PRODUCTION_BNF,
	RPA_PRODUCTION_NAMEDRULE,
	RPA_PRODUCTION_RULENAME,
	RPA_PRODUCTION_REGEXCHAR,
	RPA_PRODUCTION_CHAR,
	RPA_PRODUCTION_ESCAPEDCHAR,
	RPA_PRODUCTION_SPECIALCHAR,
	RPA_PRODUCTION_CLSCHAR,
	RPA_PRODUCTION_OCCURENCE,
	RPA_PRODUCTION_CHARRNG,
	RPA_PRODUCTION_NUMRNG,
	RPA_PRODUCTION_CLS,
	RPA_PRODUCTION_DEC,
	RPA_PRODUCTION_HEX,
	RPA_PRODUCTION_AREF,
	RPA_PRODUCTION_CREF,
	RPA_PRODUCTION_EXP,
	RPA_PRODUCTION_NOTOP,
	RPA_PRODUCTION_MINOP,
	RPA_PRODUCTION_OROP,
	RPA_PRODUCTION_ALTBRANCH,
	RPA_PRODUCTION_NEGEXP,
	RPA_PRODUCTION_NEGBRANCH,
	RPA_PRODUCTION_BRACKETEXP,
};

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
