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
