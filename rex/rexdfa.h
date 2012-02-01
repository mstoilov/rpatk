#ifndef _REXDFA_H_
#define _REXDFA_H_

#include "rex/rexdef.h"
#include "rex/rexdb.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Subset info definition
 */
typedef struct rexsi_s {
	unsigned long substate;
	rexuserdata_t userdata;
} rexsi_t;


/*
 * Transition definition
 */
typedef struct rexdft_s {
	rexchar_t lowin;
	rexchar_t highin;
	unsigned long state;
} rexdft_t;


/*
 * State definition
 */
typedef struct rexdfs_s {
	unsigned int type;
	unsigned int ntrans;
	rexdft_t *trans;
} rexdfs_t;


/*
 * Automata definition
 */
typedef struct rexdfa_s {
	unsigned long nstates;
	rexdfs_t *states;
	unsigned long ntrans;
	rexdft_t *trans;
	unsigned long nsubstates;
	rexsi_t *substates;
	unsigned long naccsubstates;
	rexsi_t *accsubstates;
} rexdfa_t;


rexdfa_t *rex_dfa_create(unsigned long nstates, unsigned long ntrans, unsigned long nsubsets, unsigned long naccsubsets);
rexdfa_t *rex_dfa_create_from_db(rexdb_t *db);
void rex_dfa_destroy(rexdfa_t *dfa);
void rex_dfa_dumpstate(rexdfa_t *dfa, long off);


#ifdef __cplusplus
}
#endif


#endif /* _REXDFA_H_ */
