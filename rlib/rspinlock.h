#ifndef _RSPINLOCK_H_
#define _RSPINLOCK_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define R_SPINLOCK_INIT 0
#define R_SPINLOCK_BUSY 1
typedef ratomic rspinlock;

void r_spinlock_init(rspinlock *lock);
void r_spinlock_lock(rspinlock *lock);
void r_spinlock_unlock(rspinlock *lock);
rboolean r_spinlock_trylock(rspinlock *lock);


#ifdef __cplusplus
}
#endif

#endif
