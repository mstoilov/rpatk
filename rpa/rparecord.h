#ifndef _RPARECORD_H_
#define _RPARECORD_H_

#include "rtypes.h"
#include "rarray.h"
#include "rlist.h"
#include "rpavm.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RPA_RECORD_NONE (0)
#define RPA_RECORD_START (1 << 0)
#define RPA_RECORD_END (1 << 1)
#define RPA_RECORD_MATCH (1 << 2)
#define RPA_RECORD_HEAD (1 << 3)
#define RPA_RECORD_TAIL (1 << 4)

#define RPA_RECORD_INVALID_UID ((ruint32)-1)

typedef struct rparecord_s {
	rlong next;
	const rchar *rule;
	const rchar *input;
	rsize_t inputsiz;
	ruint32 type;
	ruint32 top;
	ruint32 size;
	ruint32 ruleuid;
	ruint32 usertype;
	rword userdata;
} rparecord_t;


typedef rlong (*rpa_recordtree_callback)(rarray_t *records, rlong rec, rpointer userdata);

rlong rpa_recordtree_walk(rarray_t *src, rlong rec, rlong level, rpa_recordtree_callback callaback, rpointer userdata);
rlong rpa_recordtree_get(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_firstchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_lastchild(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_next(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_prev(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_parent(rarray_t *records, rlong rec, rulong type);
rlong rpa_recordtree_rotatedown(rarray_t *records, rlong parent);			/* Rotate children down, the last child becomes the first */
rlong rpa_recordtree_size(rarray_t *records, rlong rec);					/* Size of the tree */
rlong rpa_recordtree_copy(rarray_t *dst, rarray_t *src, rlong rec);
rparecord_t *rpa_record_get(rarray_t *records, rlong rec);

void rpa_record_dumpindented(rarray_t *records, rlong rec, rint level);
void rpa_record_dump(rarray_t *records, rlong rec);
rlong rpa_record_getruleuid(rarray_t *records, rlong rec);
void rpa_record_setusertype(rarray_t *records, rlong rec, ruint32 usertype, rvalset_t op);
rlong rpa_record_getusertype(rarray_t *records, rlong rec);
rint rpa_record_optchar(rparecord_t *prec, rint defc);
rint rpa_record_loopchar(rparecord_t *prec, rint defc);
#ifdef __cplusplus
}
#endif

#endif
