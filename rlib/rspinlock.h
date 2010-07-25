#ifndef _RSPINLOCK_H_
#define _RSPINLOCK_H_


#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define R_SPINLOCK_INIT 0
#define R_SPINLOCK_BUSY 1
typedef ratomic_t rspinlock_t;

void r_spinlock_init(rspinlock_t *lock);
void r_spinlock_lock(rspinlock_t *lock);
void r_spinlock_unlock(rspinlock_t *lock);
rboolean r_spinlock_trylock(rspinlock_t *lock);


#ifdef __cplusplus
}
#endif

#endif
