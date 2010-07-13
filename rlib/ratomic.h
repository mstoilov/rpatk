#ifndef _RTYPES_H_
#define _RTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


rboolean r_atomic_compare_and_exchange(volatile rint *ptr, rint oldval, rint newval);


#ifdef __cplusplus
}
#endif



#endif
