#ifndef REXJSON_H_
#define REXJSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#define REXJSON_E_NONE 0
#define REXJSON_E_UMATCHEDCB 2
#define REXJSON_E_UMATCHEDSB 3
#define REXJSON_E_NOMEM 4
#define REXJSON_E_UNEXPECTEDCHAR 5
#define REXJSON_E_MAXRECURSIONS 6

typedef enum {
	REXJSON_VALUE_NULL = 0,
	REXJSON_VALUE_TRUE,
	REXJSON_VALUE_FALSE,
	REXJSON_VALUE_INT,
	REXJSON_VALUE_NUMBER,
	REXJSON_VALUE_STRING,
	REXJSON_VALUE_ARRAY,
	REXJSON_VALUE_OBJECT,
} rexjson_valuetype_t;

typedef enum {
	REXJSON_RECORD_NONE = 0,
	REXJSON_RECORD_BEGIN = 1,
	REXJSON_RECORD_END,
} rexjson_recordtype_t;

typedef struct rexjson_record rexjson_record_t;

typedef struct rexjson_record {
	rexjson_recordtype_t rectype;
	rexjson_valuetype_t valtype;
	size_t name;			/* offset in the buffer */
	size_t namesize;		/* length of the name string */
	size_t value;			/* offset in the buffer */
	size_t valuesize;		/* length of the value string */
} rexjson_record_t;

typedef struct {
	int stacksize;
	int top;
	size_t levels;
	rexjson_record_t *stack;
	int token_id;
	const char *token_str;
	size_t token_offset;
	size_t token_size;
	size_t offset;
	size_t bufsize;
	size_t error_offset;
	int error;
	const char *buffer;
} rexjson_t;

void rexjson_init(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels);
ssize_t rexjson_parse_buffer(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels);
int rexjson_get_error(rexjson_t *ctx);
size_t rexjson_get_error_offset(rexjson_t *ctx);
ssize_t rexjson_recordtree_firstchild(rexjson_record_t* recs, size_t recsize, ssize_t rec_index);
ssize_t rexjson_recordtree_next(rexjson_record_t* recs, size_t recsize, ssize_t rec_index);
void rexjson_dump_tree(FILE *file, rexjson_record_t* recs, size_t recsize, size_t record, const char* buffer, int indent);
void rexjson_dump_records(FILE *file, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize);

#ifdef __cplusplus
}
#endif

#endif /* REXJSON_H_ */
