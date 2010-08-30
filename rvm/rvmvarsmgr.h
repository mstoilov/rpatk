#ifndef _RVMVARSMGR_H_
#define _RVMVARSMGR_H_

#include "rtypes.h"
#include "rarray.h"
#include "rhash.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rvm_varsmgr_s {
	rarray_t *names;
	rhash_t *nameshash;
} rvm_varsmgr_t;


rvm_varsmgr_t *rvm_varsmgr_create();
void rvm_varsmgr_destroy(rvm_varsmgr_t *varsmgr);
void rvm_varsmgr_addvar(rvm_varsmgr_t *varsmgr, const rchar* varname);




#ifdef __cplusplus
}
#endif

#endif
