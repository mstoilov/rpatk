/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
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
 * @file rextransition.h
 * @brief Definitions related to NFA/DFA transitions
 *
 *
 * <h2>Synopsis</h2>
 * The following definitions are implementation of NFA/DFA transitions.
 */


#ifndef _REXTRANSITION_H_
#define _REXTRANSITION_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	REX_TRANSITION_INPUT = 0,
	REX_TRANSITION_RANGE = 1,
	REX_TRANSITION_EMPTY = 2
} rex_transitiontype_t;


/*
 * Definition of transition structure. The transition type will
 * determine which members are valid.
 *
 */
typedef struct rex_transition_s {
	unsigned int type;
	unsigned int lowin;
	unsigned int highin;
	unsigned long srcuid;
	unsigned long dstuid;
} rex_transition_t;


#ifdef __cplusplus
}
#endif

#endif
