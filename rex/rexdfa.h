/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@rpasearch.com>
 */

/**
 * @file rex/rexdfa.h
 * @brief Definition of DFA interface
 *
 *
 * <h2>Synopsis</h2>
 * This file defines the data structures and the macros for the DFA implementation.
 */

#ifndef _REXDFA_H_
#define _REXDFA_H_

#include "rexdfatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REX_DFA_HASHBITS(__bytes__, __bitsperbyte__) ((__bytes__) * (__bitsperbyte__))						/* Number of hashbits */
#define REX_DFA_HASHSIZE(__bytes__, __bitsperbyte__) (1 << REX_DFA_HASHBITS(__bytes__, __bitsperbyte__))	/* The size of the bit array in bits. */
#define REX_DFA_HASHMASK(__bytes__, __bitsperbyte__) (REX_DFA_HASHSIZE(__bytes__, __bitsperbyte__) - 1)		/* Mask with the number of hash bits set to 1 */

#define REX_BITARRAY_BYTE(__arr__, __entry__) (((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3])
#define REX_BITARRAY_GET(__arr__, __entry__) ((((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] & (1 << (((unsigned int)(__entry__)) & 0x7))) ? 1 : 0)
#define REX_BITARRAY_SET(__arr__, __entry__) do { ((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] |= (1 << (((unsigned int)(__entry__)) & 0x7)); } while (0)
#define REX_BITARRAY_CLR(__arr__, __entry__) do { ((unsigned char*)__arr__)[((unsigned int)(__entry__))>>3] &= ~(1 << (((unsigned int)(__entry__)) & 0x7)); } while (0)


rexdfa_t *rex_dfa_create(rexuint_t nstates, rexuint_t ntrans, rexuint_t naccsubstates, rexuint_t nsubstates);
void rex_dfa_destroy(rexdfa_t *dfa);
int rex_dfa_hash(rexdfa_t *dfa, unsigned int hbytes, unsigned int hbits);
void rex_dfa_dumpstate(rexdfa_t *dfa, rexuint_t nstate);
rexdfs_t *rex_dfa_state(rexdfa_t *dfa, rexuint_t nstate);
rexdft_t *rex_dfa_transition(rexdfa_t *dfa, rexuint_t nstate, rexuint_t ntransition);
rexdfss_t *rex_dfa_substate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t nsubstate);
rexdfss_t *rex_dfa_accsubstate(rexdfa_t *dfa, rexuint_t nstate, rexuint_t naccsubstate);
rexuint_t rex_dfa_next(rexdfa_t *dfa, rexuint_t nstate, rexchar_t input);

#ifdef __cplusplus
}
#endif


#endif /* _REXDFA_H_ */
