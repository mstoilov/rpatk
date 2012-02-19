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

#ifndef _RBUFFER_H_
#define _RBUFFER_H_

#include "rtypes.h"
#include "rlib/robject.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rbuffer_s rbuffer_t;
typedef void (*R_BUFFER_DESTROY)(rbuffer_t *buffer);

struct rbuffer_s {
	char *s;
	unsigned long size;
	void *userdata;
	void *userdata1;
	R_BUFFER_DESTROY alt_destroy;
};

void r_buffer_destroy(rbuffer_t *buffer);
rbuffer_t *r_buffer_create(unsigned long size);
int r_buffer_append(rbuffer_t *buffer, void *src, unsigned long size);


#ifdef __cplusplus
}
#endif

#endif /* _RBUFFER_H_ */
