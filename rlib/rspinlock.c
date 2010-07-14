#include "rspinlock.h"
#include "ratomic.h"


void r_spinlock_init(rspinlock *lock)
{
	*lock = 0;
}

void r_spinlock_lock(rspinlock *lock)
{
	while (1)
	{
		if (!r_atomic_exchange(lock, R_SPINLOCK_BUSY))
			return;

		while (*lock);
	}
}


void r_spinlock_unlock(rspinlock *lock)
{
	*lock = 0;
}


rboolean r_spinlock_trylock(rspinlock *lock)
{
	return (!r_atomic_exchange(lock, R_SPINLOCK_BUSY));
}
