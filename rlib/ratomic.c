#include "rtypes.h"


rboolean r_atomic_compare_and_exchange(volatile ratomic *ptr, ratomic oldval, ratomic newval)
{
	ratomic result;

	R_ATOMIC_CMPXCHG(ptr, oldval, newval, &result);
	return (result == oldval);
}


ratomic r_atomic_exchange(volatile ratomic *ptr, ratomic val)
{
	R_ATOMIC_XCHG(ptr, val);
	return val;
}


void r_atomic_add(volatile ratomic *ptr, ratomic val)
{
	R_ATOMIC_ADD(ptr, val);
}


void r_atomic_sub(volatile ratomic *ptr, ratomic val)
{
	R_ATOMIC_SUB(ptr, val);
}
