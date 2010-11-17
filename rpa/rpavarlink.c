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

#include "rpavarlink.h"
#include "rmem.h"
#include "rstring.h"
#include "rpamatch.h"
#include "rpamatchstr.h"
#include "rpamatchrange.h"
#include "rpamatchval.h"
#include "rpamatchlist.h"



rpa_varlink_t *rpa_varlink_create(unsigned char type, const char *name)
{
	rpa_varlink_t *pVarLink;

	pVarLink = (rpa_varlink_t*)r_malloc(sizeof(*pVarLink));
	if (!pVarLink)
		return ((void*)0);
	rpa_list_init(&pVarLink->lnk);
	rpa_list_init(&pVarLink->hlnk);
	rpa_var_init(&pVarLink->var, type, name);
	return pVarLink;
}


void rpa_varlink_destroy(rpa_varlink_t *pVarLink)
{
	if (!pVarLink)
		return;
	if (!rpa_list_empty(&pVarLink->lnk))
		rpa_list_del(&pVarLink->lnk);
	if (!rpa_list_empty(&pVarLink->hlnk))
		rpa_list_del(&pVarLink->hlnk);		
	rpa_var_finalize(&pVarLink->var);
	r_free((void*)pVarLink);
}


void rpa_varlink_destroy_all(rpa_head_t *head)
{
	while (!rpa_list_empty(head))
		rpa_varlink_destroy(rpa_list_entry(rpa_list_first(head), rpa_varlink_t, lnk));
}
