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

#include "rtypes.h"
#include "rlib/ratomic.h"


rboolean r_atomic_compare_and_exchange(volatile ratomic_t *ptr, ratomic_t oldval, ratomic_t newval)
{
	volatile ratomic_t res;

	R_ATOMIC_CMPXCHG(ptr, oldval, newval, res);
	return (res == oldval);
}


ratomic_t r_atomic_exchange(volatile ratomic_t *ptr, volatile ratomic_t val)
{
	R_ATOMIC_XCHG(ptr, val);
	return val;
}


ratomic_t r_atomic_add(volatile ratomic_t *ptr, ratomic_t val)
{
	ratomic_t res = 0;
	R_ATOMIC_ADD(ptr, val, res);
	return res;
}


ratomic_t r_atomic_sub(volatile ratomic_t *ptr, ratomic_t val)
{
	ratomic_t res = 0;
	R_ATOMIC_SUB(ptr, val, res);
	return res;
}


ratomic_t r_atomic_get(volatile ratomic_t *ptr)
{
	ratomic_t res;
	R_ATOMIC_GET(ptr, res);
	return res;
}


void r_atomic_set(volatile ratomic_t *ptr, ratomic_t val)
{
	R_ATOMIC_SET(ptr, val);
}
