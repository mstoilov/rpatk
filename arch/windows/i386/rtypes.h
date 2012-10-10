#ifndef _RTYPES_H_
#define _RTYPES_H_
#include <windows.h>

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
typedef signed long long rint64;
typedef unsigned long long ruint64;
typedef long ratomic_t;
typedef unsigned long ruword;
typedef long rword;

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
		do { res = InterlockedCompareExchange (ptr, newval, oldval); } while (0)

#define R_ATOMIC_XCHG(ptr, val) \
		do { val = InterlockedExchange (ptr, val); } while (0)

#define R_ATOMIC_ADD(ptr, val) \
		do { InterlockedExchangeAdd (ptr, val); } while (0)

#define R_ATOMIC_SUB(ptr, val) \
		do { InterlockedExchangeSubtract (ptr, val); } while (0)

#define R_ATOMIC_GET(ptr, res) \
	do { res = *ptr; } while (0)

#define R_ATOMIC_SET(ptr, val) \
	do { *ptr = val; } while (0)


#define R_DEBUG_BRAKE do { __asm int 3 } while (0)
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

