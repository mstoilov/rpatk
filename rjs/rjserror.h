#ifndef _RJSERROR_H_
#define _RJSERROR_H_


#define RJS_ERRORTYPE_NONE 0
#define RJS_ERRORTYPE_SYNTAX 1
#define RJS_ERRORTYPE_RUNTIME 2


#define RJS_ERROR_NONE 0
#define RJS_ERROR_UNDEFINED 1
#define RJS_ERROR_SYNTAX 2
#define RJS_ERROR_NOTAFUNCTION 3
#define RJS_ERROR_NOTAFUNCTIONCALL 4
#define RJS_ERROR_NOTALOOP 5
#define RJS_ERROR_NOTAIFSTATEMENT 6


typedef struct rjs_error_s {
	rlong type;
	rlong error;
	rlong offset;
	rlong size;
	rlong line;
	rlong lineoffset;
	const rchar *script;
} rjs_error_t;


#endif /* RJSERROR_H_ */
