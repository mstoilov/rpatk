#ifndef _RTYPES_H_
#define _RTYPES_H_

#include <stdint.h>
#include <assert.h>
#include "stddef.h"

/* 
 * Architecture dependent types. These types have to be redefined
 * for every architecture
 */
typedef int8_t rint8;
typedef uint8_t ruint8;
typedef int16_t rint16;
typedef uint16_t ruint16;
typedef int32_t rint32;
typedef uint32_t ruint32;
typedef int64_t rint64;
typedef uint64_t ruint64;
typedef uint64_t ruword;
typedef int64_t rword;
typedef uint32_t ratomic_t;


/*
 * Common types. These types should be the same for most of the architectures.
 */
typedef uint32_t rboolean;
typedef void *rpointer;
typedef const void *rconstpointer;
typedef struct {ruint32 p1; ruint32 p2;} rpair_t;

/*
 * Atomic operations (Architecture Dependent)
 */

/*
 * If the current value of *ptr is oldval, then write newval into *ptr.
 */
#define R_ATOMIC_CMPXCHG(ptr, oldval, newval, res) \
	do { res = __sync_val_compare_and_swap(ptr, oldval, newval); } while (0)

/*
 * Writes value into *ptr, and returns the previous contents of *ptr.
 */
#define R_ATOMIC_XCHG(ptr, val) \
	do { val = __sync_lock_test_and_set(ptr, val); } while (0)

/*
 * { tmp = *ptr; *ptr += value; return tmp; }
 */
#define R_ATOMIC_ADD(ptr, val, res) \
	do { res = __sync_fetch_and_add(ptr, val); } while (0)

/*
 * { tmp = *ptr; *ptr -= value; return tmp; }
 */
#define R_ATOMIC_SUB(ptr, val, res) \
	do { res = __sync_fetch_and_sub(ptr, val); } while (0)

#define R_ATOMIC_GET(ptr, res) \
	do { __sync_synchronize (); res = *ptr; } while (0)

#define R_ATOMIC_SET(ptr, val) \
	do { *ptr = val; __sync_synchronize (); } while (0)

#define R_DEBUG_BRAKE assert(0)
#define R_ASSERT(__a__) do { if (!(__a__)) R_DEBUG_BRAKE; } while (0)
#define R_SIZE_ALIGN(s, n) ((((s) + (n) - 1) / (n)) * (n))
#define R_MIN(a, b) ((a) < (b) ? (a): (b))
#define R_MAX(a, b) ((a) > (b) ? (a): (b))

#ifndef NULL
#define NULL ((rpointer)0)
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

