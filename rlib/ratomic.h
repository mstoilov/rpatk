#ifndef _RATOMIC_H_
#define _RATOMIC_H_

#ifdef __cplusplus
extern "C" {
#endif


rboolean r_atomic_compare_and_exchange(volatile ratomic_t *ptr, ratomic_t oldval, ratomic_t newval);
ratomic_t r_atomic_exchange(volatile ratomic_t *ptr, volatile ratomic_t val);
void r_atomic_add(volatile ratomic_t *ptr, ratomic_t val);
void r_atomic_sub(volatile ratomic_t *ptr, ratomic_t val);

#ifdef __cplusplus
}
#endif



#endif
