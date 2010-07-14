#ifndef _RATOMIC_H_
#define _RATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif


rboolean r_atomic_compare_and_exchange(volatile ratomic *ptr, ratomic oldval, ratomic newval);
ratomic r_atomic_exchange(volatile ratomic *ptr, ratomic val);
ratomic r_atomic_add(volatile ratomic *ptr, ratomic val);
ratomic r_atomic_sub(volatile ratomic *ptr, ratomic val);

#ifdef __cplusplus
}
#endif



#endif
