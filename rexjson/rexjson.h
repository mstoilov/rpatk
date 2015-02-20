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

/**
 * \struct rexjson_record <rexjson.h> <rexjson.h>
 * This structure is used to build the parsed JSON tree
 */
typedef struct rexjson_record {
	rexjson_recordtype_t rectype;	/* This can one of: REXJSON_RECORD_NONE, REXJSON_RECORD_BEGIN, REXJSON_RECORD_END */
	rexjson_valuetype_t valtype;	/* This is the JSON value type: REXJSON_VALUE_NULL, REXJSON_VALUE_TRUE, ... , REXJSON_VALUE_OBJECT */
	size_t name;					/* offset in the buffer */
	size_t namesize;				/* length of the name string */
	size_t value;					/* offset in the buffer */
	size_t valuesize;				/* length of the value string */
} rexjson_record_t;

/**
 * \struct rexjson_t <rexjson.h> <rexjson.h>
 * This structure is the parsing context.
 */
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

/**
 * This function is called by @ref rexjson_parse_buffer and shouldn't be called directly.
 */
void rexjson_init(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels);

/**
 * Parse the buffer string and fill the recs array with records representing the parsed JSON tree.
 * To see how this tree looks like you can use @ref rexjson_dump_tree
 *
 * @param ctx pointer to parse context structure. It doesn't need to be initialized.
 * @param recs array of records, representing the parsed JSON. This array will be filled if parsing is successful.
 * @param recsize the size of the records array in number of records (not bytes).
 * @param buffer pointer to the buffer containing the JSON string
 * @param bufsize size of the buffer.
 * @param maxlevels This parameters controls how deep the JSON tree can be.
 * @return If successful the function returns the size of the parsed string in the @ref buffer. Returns negative if fails.
 */
ssize_t rexjson_parse_buffer(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels);

/**
 * Return the error code if @ref rexjson_parse_buffer fails (returns negative)
 */
int rexjson_get_error(rexjson_t *ctx);

/**
 * Return the offset at which @ref rexjson_parse_buffer encountered error
 */
size_t rexjson_get_error_offset(rexjson_t *ctx);
/**
 * Return the first child.
 * @param recs array of records, representing the parsed JSON.
 * @param recsize the size of the records array in number of records (not bytes).
 * @param rec_index The index of the current record, in other words the parent of the first child. The root is located at index 0.
 * @return first child of the current record @ref rec_index.
 */
ssize_t rexjson_recordtree_firstchild(rexjson_record_t* recs, size_t recsize, ssize_t rec_index);

/**
 * Return the next sibling.
 * @param recs array of records, representing the parsed JSON.
 * @param recsize the size of the records array in number of records (not bytes).
 * @param rec_index The index of the current record.
 * @return next sibling of the current record @ref rec_index.
 */
ssize_t rexjson_recordtree_next(rexjson_record_t* recs, size_t recsize, ssize_t rec_index);

/**
 * Dump the parsed JSON tree in a file. This function is used for debugging.
 * @param file Dump to this file.
 * @recs  array of records, representing the parsed JSON.
 * @param recsize the size of the records array in number of records (not bytes).
 * @param start_record_index the root of the JSON tree. Most of the time you would want to start from the root, so this will be 0
 * @param buffer the JSON string buffer that was passed to @ref rexjson_parse_buffer
 * @param indent starting indentation, most of the time will be 0
 * @return none
 */
void rexjson_dump_tree(FILE *file, rexjson_record_t* recs, size_t recsize, size_t start_record_index, const char* buffer, int indent);

/**
 * Dump the parsed JSON records in a file. This function is used for debugging.
 * @param file Dump to this file.
 * @recs  array of records, representing the parsed JSON.
 * @param recsize the size of the records array in number of records (not bytes).
 * @param buffer the JSON string buffer that was passed to @ref rexjson_parse_buffer
 * @return none
 */
void rexjson_dump_records(FILE *file, rexjson_record_t* recs, size_t recsize, const char* buffer);

#ifdef __cplusplus
}
#endif

#endif /* REXJSON_H_ */
