#ifndef _RTYPES_H_
#define _RTYPES_H_

/* 
 * Architecture dependent types. These types have to be redefined
 * for every architecture
 */
typedef signed char rint8;
typedef unsigned char ruint8;
typedef signed short rint16;
typedef unsigned short ruint16;
typedef signed int rint32;
typedef unsigned int ruint32;
typedef signed long rint64;
typedef unsigned long ruint64;
typedef unsigned long ruword;
typedef long rword;
typedef unsigned int ratomic_t;


/*
 * Common types. These types should be the same for most of the architectures.
 */
typedef int rboolean;
typedef void *rpointer;
typedef const void *rconstpointer;
typedef struct {ruint32 p1; ruint32 p2;} rpair_t;

/*
 * Atomic operations (Architecture Dependent)
 */
#define R_ATOMIC_CMPXCHG(ptr, oldval, newval, res) \
	do { res = __sync_val_compare_and_swap(ptr, oldval, newval); } while (0)

#define R_ATOMIC_XCHG(ptr, val) \
	do { val = __sync_lock_test_and_set(ptr, val); } while (0)

#define R_ATOMIC_ADD(ptr, val, res) \
	do { res = __sync_fetch_and_add(ptr, val); } while (0)

#define R_ATOMIC_SUB(ptr, val, res) \
	do { res = __sync_fetch_and_sub(ptr, val); } while (0)

#define R_ATOMIC_GET(ptr, res) \
	do { __sync_synchronize (); res = *ptr; } while (0)

#define R_ATOMIC_SET(ptr, val) \
	do { *ptr = val; __sync_synchronize (); } while (0)

#define R_DEBUG_BRAKE __asm__ ("int $3")
#define R_ASSERT(__a__) do {if (!(__a__)) R_DEBUG_BRAKE; } while (0)
#define R_SIZE_ALIGN(s, n) ((((s) + (n) - 1) / (n)) * (n))
#define R_MIN(a, b) ((a) < (b) ? (a): (b))
#define R_MAX(a, b) ((a) > (b) ? (a): (b))

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((rpointer)0)
#endif
#endif

#ifndef TRUE
#define TRUE ((rboolean)1)
#endif

#ifndef FALSE
#define FALSE ((rboolean)0)
#endif


typedef enum {
	RVALSET_NONE = 0,
	RVALSET_OR,
	RVALSET_XOR,
	RVALSET_AND,
} rvalset_t;


#endif

