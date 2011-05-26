#ifndef _RJSPARSER_H_
#define _RJSPARSER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rarray.h"
#include "rvmcpu.h"
#include "rpa.h"
#include "rjserror.h"


typedef struct rjs_parser_s {
	rpadbex_t *dbex;

} rjs_parser_t;


rjs_parser_t *rjs_parser_create();
void rjs_parser_destroy(rjs_parser_t *parser);
rlong rjs_parser_exec(rjs_parser_t *parser, const rchar *script, rsize_t size, rarray_t *ast, rjs_error_t *error);
rlong rjs_parser_offset2line(const rchar *script, rlong offset);
rlong rjs_parser_offset2lineoffset(const rchar *script, rlong offset);


#ifdef __cplusplus
}
#endif

#endif
