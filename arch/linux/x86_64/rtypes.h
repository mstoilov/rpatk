#ifndef _RTYPES_H_
#define _RTYPES_H_

/*
 * Common types. These types should be the same for most of the architectures.
 */
typedef long rlong;
typedef int rint;
typedef short rshort;
typedef char rchar;
typedef unsigned long rulong;
typedef unsigned int ruint;
typedef unsigned short rushort;
typedef unsigned char ruchar;
typedef double rdouble;
typedef float rfloat;
typedef rint rboolean;
typedef void *rpointer;
typedef const void *rconstpointer;


/* 
 * Architecture dependent types. These types have to be redifined 
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
typedef unsigned long rword;
typedef unsigned long rsize_t;
typedef signed long rssize_t;
typedef unsigned int ratomic;

/*
 * Atomic operations (Architecture Dependent)
 */
#define R_ATOMIC_CMPXCHG(ptr, oldval, newval, resptr) \
		do { __asm__ __volatile__ ("lock; cmpxchgl %2, %1" \
			: "=a" (*(resptr)), "=m" (*ptr) \
			: "r" (newval), "m" (*ptr), "0" (oldval)); } while (0)

#define R_ATOMIC_XCHG(ptr, val) \
		do { __asm__ __volatile__("xchgl %0,%1" \
			:"=r" ((ruint32) val) \
			:"m" (*(volatile ruint32 *)ptr), "0" (val) \
			:"memory"); } while (0)

#define R_ATOMIC_ADD(ptr, val) \
		do { __asm__ __volatile__ ("addl %1,%0" \
			: "=m" (*ptr) \
			: "ir" (val), "m" (*ptr)); } while (0)

#define R_ATOMIC_SUB(ptr, val) \
		do { __asm__ __volatile__ ("subl %1,%0" \
			: "=m" (*ptr) \
			: "ir" (val), "m" (*ptr)); } while (0)



#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((rpointer)0)
#endif
#endif

#endif

