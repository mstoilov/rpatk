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

#include "rlib/rspinlock.h"
#include "rlib/ratomic.h"


void r_spinlock_init(rspinlock_t *lock)
{
	*lock = 0;
}

void r_spinlock_lock(rspinlock_t *lock)
{
	while (1)
	{
		if (!r_atomic_exchange(lock, R_SPINLOCK_BUSY))
			return;

		while (*lock);
	}
}


void r_spinlock_unlock(rspinlock_t *lock)
{
	*lock = 0;
}


rboolean r_spinlock_trylock(rspinlock_t *lock)
{
	return (!r_atomic_exchange(lock, R_SPINLOCK_BUSY));
}
