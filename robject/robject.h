#ifndef _ROBJECT_H_
#define _ROBJECT_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

rint r_object_major_version();
rint r_object_minor_version();

typedef enum _RVariantType {
	R_TYPE_NONE = 0,
	R_TYPE_LONG,
	R_TYPE_INT,
	R_TYPE_SHORT,
	R_TYPE_CHAR,
	R_TYPE_STRING,
	R_TYPE_POINTER,
} RVariantType;
	

typedef struct _RString {
	rchar *ptr;
	rulong size;
} RString;


typedef struct _RVariant {
	RVariantType type;
	union {
		rlong v_long;
		rint v_int;
		rshort v_short;
		rchar v_char;
		rpointer v_pointer;
		RString v_string;
	} data;
} RVariant;


#ifdef __cplusplus
}
#endif

#endif
