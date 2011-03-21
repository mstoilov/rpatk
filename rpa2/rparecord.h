#ifndef _RPARECORD_H_
#define _RPARECORD_H_

#include "rtypes.h"
#include "rarray.h"
#include "rlist.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RECORD_NONE (0)
#define RPA_RECORD_START (1 << 0)
#define RPA_RECORD_END (1 << 1)
#define RPA_RECORD_MATCH (1 << 2)

#define RPA_RECORD_INVALID_UID ((ruint32)-1)

typedef struct rparecord_s {
	rlist_t head;
	rlink_t lnk;
	const char *rule;
	ruint32 userid;
	ruint32 ruleid;
	ruint32 top;
	ruint32 size;
	ruint32 type;
	const char *input;
	rsize_t inputsiz;
} rparecord_t;


rlong rpa_recordtree_get(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_firstchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_lastchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_next(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_prev(rarray_t *records, rlong rec, rulong type);

void rpa_record_dump(rarray_t *records, rlong rec);

#ifdef __cplusplus
}
#endif

#endif
