#include "rtypes.h"
#include "ratomic.h"


rboolean r_atomic_compare_and_exchange(volatile ratomic_t *ptr, ratomic_t oldval, ratomic_t newval)
{
	volatile ratomic_t result;

	R_ATOMIC_CMPXCHG(ptr, oldval, newval, &result);
	return (result == oldval);
}


ratomic_t r_atomic_exchange(volatile ratomic_t *ptr, volatile ratomic_t val)
{
	R_ATOMIC_XCHG(ptr, val);
	return val;
}


void r_atomic_add(volatile ratomic_t *ptr, ratomic_t val)
{
	R_ATOMIC_ADD(ptr, val);
}


void r_atomic_sub(volatile ratomic_t *ptr, ratomic_t val)
{
	R_ATOMIC_SUB(ptr, val);
}
