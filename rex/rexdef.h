/*
 * rexdef.h
 *
 *  Created on: Jan 31, 2012
 *      Author: mstoilov
 */

#ifndef _REXDEF_H_
#define _REXDEF_H_

#ifndef REX_USERDATA_TYPE
typedef unsigned long rexuserdata_t;
#else
typedef REX_USERDATA_TYPE rexuserdata_t;
#endif

#ifndef REX_CHAR_TYPE
typedef unsigned int rexchar_t;
#else
typedef REX_CHAR_TYPE rexchar_t;
#endif
#define REX_CHAR_MAX ((rexchar_t)-1)
#define REX_CHAR_MIN ((rexchar_t)0)


#endif /* _REXDEF_H_ */
