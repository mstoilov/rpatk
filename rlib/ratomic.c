#include "rtypes.h"


rboolean r_atomic_compare_and_exchange(volatile rint *ptr, rint oldval, rint newval)
{
	rint result;

	R_COMPARE_AND_EXCHANGE(ptr, oldval, newval, result);
	return (result == oldval);
}
