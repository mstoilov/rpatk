#ifndef _RJSPARSER_H_
#define _RJSPARSER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rarray.h"
#include "rvmcpu.h"
#include "rpadbex.h"
#include "rpastat.h"
#include "rpaerror.h"


typedef struct rjs_parser_s {
	rpadbex_t *dbex;

} rjs_parser_t;


rjs_parser_t *rjs_parser_create();
void rjs_parser_destroy(rjs_parser_t *parser);
rlong rjs_parser_exec(rjs_parser_t *parser, const rchar *script, rsize_t size, rarray_t **ast);


#ifdef __cplusplus
}
#endif

#endif
