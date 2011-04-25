#ifndef _RJS_H_
#define _RJS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtypes.h"
#include "rjsparser.h"

#define RJS_VERSION_MAJOR 0
#define RJS_VERSION_MINOR 51
#define RJS_VERSION_MICRO 1
#define RJS_VERSION_STRING "0.51.1"

const rchar *rjs_version();


#ifdef __cplusplus
}
#endif

#endif
