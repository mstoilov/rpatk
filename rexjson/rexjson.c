/*
 * To generate the header file rexjsondfa.h from rexjsondfa.rexcc do:
 * # rexcc rexjsondfa.rexcc -o rexjsondfa.h
 * 
 * To build the test program:
 * # gcc -o testrexjson rexjson.c -DREXJSON_TEST_MAIN
 *
 * To use the library in your own project, just add these 3 files to your project:
 * rexjson.c
 * rexjson.h
 * rexjsondfa.h  (generated from rexjsondfa.rexcc)
 * rexdfatypes.h
 */

#include <string.h>
#include <stdio.h>
#include "rexjson.h"
#include "rexjsondfa.h"


static ssize_t rexjson_parse_value(rexjson_t* ctx);
static ssize_t rexjson_parse_value_set_name(rexjson_t* ctx, size_t nameoffset, size_t namesize);

static rexjson_record_t *rexjson_record_add(rexjson_t* ctx, rexjson_recordtype_t rectype, rexjson_valuetype_t valtype)
{
	if (ctx->top >= ctx->stacksize) {
		ctx->error = REXJSON_E_NOMEM;
		return NULL;
	}
	memset(&ctx->stack[ctx->top], 0, sizeof(rexjson_record_t));
	ctx->stack[ctx->top].rectype = rectype;
	ctx->stack[ctx->top].valtype = valtype;
	ctx->top++;
	return &ctx->stack[ctx->top - 1];
}

static rexjson_record_t *rexjson_record_current(rexjson_t* ctx)
{
	if (ctx->top <= 0)
		return NULL;
	return &ctx->stack[ctx->top - 1];
}

static void rexjson_get_token(rexjson_t *ctx)
{
	rexdfss_t *acc_ss = NULL;
	rexuint_t nstate = REX_DFA_STARTSTATE;
	int ch, i;
	const char *buffer = &ctx->buffer[ctx->offset];
	int size = ctx->bufsize - ctx->offset;

	ctx->token_str = buffer;
	ctx->token_size = 0;
	ctx->token_id = -1;
	for (i = 0; i < size; i++) {
		ch = buffer[i];
		REX_DFA_NEXT(dfa, nstate, ch, &nstate);
		if (nstate == REX_DFA_DEADSTATE) {
			break;
		}
		if (REX_DFA_STATE(dfa, nstate)->type == REX_STATETYPE_ACCEPT) {
			/*
			 * Note: We will not break out of the loop here. We will keep going
			 * in order to find the longest match.
			 */
			acc_ss = REX_DFA_ACCSUBSTATE(dfa, nstate, 0);
			ctx->token_size = i + 1;
			ctx->token_id = (int) acc_ss->userdata;
		}
	}
	if (ctx->token_size > 0) {
		ctx->token_offset = ctx->offset;
		ctx->offset += ctx->token_size;
	}
}

static void rexjson_next_token(rexjson_t *ctx)
{
	rexjson_get_token(ctx);
	while (ctx->token_id == TOKEN_SPACE)
		rexjson_get_token(ctx);
}

static ssize_t rexjson_error_unexpectedchar(rexjson_t* ctx)
{
	ctx->error_offset = ctx->token_offset;
	ctx->error = REXJSON_E_UNEXPECTEDCHAR;
#ifdef REXJSON_PRINT_UNEXPECTEDCHAR
	fprintf(stdout, "Unexpected: '%.*s' at %d\n", (int)ctx->token_size, ctx->token_str, (int)(ctx->error_offset));
#endif
	return -1;
}

static ssize_t rexjson_parse_token(rexjson_t* ctx, int token)
{
	size_t token_size = ctx->token_size;
	if (ctx->token_id != token) {
		return rexjson_error_unexpectedchar(ctx);
	}
	rexjson_next_token(ctx);
	return token_size;
}

static ssize_t rexjson_parse_name(rexjson_t* ctx)
{
	size_t cur_offset = ctx->token_offset;

	if (rexjson_parse_token(ctx, TOKEN_STRING) < 0) {
		return -1;
	}
	return (ctx->token_offset - cur_offset);
}

static ssize_t rexjson_parse_namevalue(rexjson_t* ctx)
{
	size_t cur_offset = ctx->token_offset;
	size_t name_offset = 0, name_size = 0;

	name_offset = ctx->token_offset;
	name_size = ctx->token_size;
	if (rexjson_parse_name(ctx) < 0)
		return -1;
	if (rexjson_parse_token(ctx, TOKEN_COLON) < 0)
		return -1;
	if (rexjson_parse_value_set_name(ctx, name_offset, name_size) < 0)
		return -1;
	return (ctx->token_offset - cur_offset);
}


static ssize_t rexjson_parse_object(rexjson_t* ctx)
{
	size_t cur_offset = ctx->token_offset;
	rexjson_record_t *record = rexjson_record_current(ctx);

	if (rexjson_parse_token(ctx, TOKEN_LEFTCB) < 0)
		return -1;
	if (ctx->token_id != TOKEN_RIGHTCB) {
		if (rexjson_parse_namevalue(ctx) < 0)
			return -1;
		while (ctx->token_id == TOKEN_COMMA) {
			if (rexjson_parse_token(ctx, TOKEN_COMMA) < 0)
				return -1;
			if (rexjson_parse_namevalue(ctx) < 0)
				return -1;
		}
	}
	if (rexjson_parse_token(ctx, TOKEN_RIGHTCB) < 0)
		return -1;
	record->value = cur_offset;
	record->valuesize = ctx->token_offset - cur_offset;
	record->valtype = REXJSON_VALUE_OBJECT;
	return (ctx->token_offset - cur_offset);
}

static ssize_t rexjson_parse_array(rexjson_t* ctx)
{
	size_t cur_offset = ctx->token_offset;
	rexjson_record_t *record = rexjson_record_current(ctx);

	if (rexjson_parse_token(ctx, TOKEN_LEFTSB) < 0)
		return -1;
	if (ctx->token_id != TOKEN_RIGHTSB) {
		if (rexjson_parse_value(ctx) < 0)
			return -1;
		while (ctx->token_id == TOKEN_COMMA) {
			if (rexjson_parse_token(ctx, TOKEN_COMMA) < 0)
				return -1;
			if (rexjson_parse_value(ctx) < 0)
				return -1;
		}
	}
	if (rexjson_parse_token(ctx, TOKEN_RIGHTSB) < 0)
		return -1;
	record->value = cur_offset;
	record->valuesize = ctx->token_offset - cur_offset;
	record->valtype = REXJSON_VALUE_ARRAY;
	return (ctx->token_offset - cur_offset);
}


static ssize_t rexjson_parse_primitive(rexjson_t* ctx)
{
	rexjson_record_t *rec = rexjson_record_current(ctx);
	size_t ret = ctx->token_size;
	switch (ctx->token_id) {
	case TOKEN_STRING:
		rec->valtype = REXJSON_VALUE_STRING;
		break;
	case TOKEN_INT:
		rec->valtype = REXJSON_VALUE_INT;
		break;
	case TOKEN_NUMBER:
		rec->valtype = REXJSON_VALUE_NUMBER;
		break;
	case TOKEN_NULL:
		rec->valtype = REXJSON_VALUE_NULL;
		break;
	case TOKEN_TRUE:
		rec->valtype = REXJSON_VALUE_TRUE;
		break;
	case TOKEN_FALSE:
		rec->valtype = REXJSON_VALUE_FALSE;
		break;
	default:
		return rexjson_error_unexpectedchar(ctx);
	}
	rec->value = ctx->token_offset;
	rec->valuesize = ctx->token_size;
	rexjson_next_token(ctx);
	return ret;
}

static ssize_t rexjson_parse_value(rexjson_t* ctx)
{
	return rexjson_parse_value_set_name(ctx, 0, 0);
}

static ssize_t rexjson_parse_value_set_name(rexjson_t* ctx, size_t nameoffset, size_t namesize)
{
	ssize_t ret = -1;
	rexjson_record_t *end, *begin;

	if (!ctx->levels) {
		ctx->error_offset = ctx->token_offset;
		ctx->error = REXJSON_E_MAXRECURSIONS;
		return -1;
	}
	ctx->levels--;
	switch (ctx->token_id) {
	case TOKEN_LEFTCB:
		if ((begin = rexjson_record_add(ctx, REXJSON_RECORD_BEGIN, REXJSON_VALUE_OBJECT)) == NULL)
			return -1;
		ret = rexjson_parse_object(ctx);
		break;
	case TOKEN_LEFTSB:
		if ((begin = rexjson_record_add(ctx, REXJSON_RECORD_BEGIN, REXJSON_VALUE_ARRAY)) == NULL)
			return -1;
		ret = rexjson_parse_array(ctx);
		break;
	default:
		if ((begin = rexjson_record_add(ctx, REXJSON_RECORD_BEGIN, REXJSON_VALUE_NULL)) == NULL)
			return -1;
		ret = rexjson_parse_primitive(ctx);
	}
	begin->name = nameoffset;
	begin->namesize = namesize;
	if ((end = rexjson_record_add(ctx, REXJSON_RECORD_END, REXJSON_VALUE_NULL)) == NULL)
		return -1;
	*end = *begin;
	end->rectype = REXJSON_RECORD_END;
	ctx->levels++;
	return ret;
}

void rexjson_init(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->buffer = buffer;
	ctx->bufsize = bufsize;
	ctx->stack = recs;
	ctx->stacksize = recsize;
	ctx->levels = maxlevels;
}

int rexjson_get_error(rexjson_t *ctx)
{
	return ctx->error;
}

size_t rexjson_get_error_offset(rexjson_t *ctx)
{
	return ctx->error_offset;
}

static void rexjson_record_dump(FILE* file, rexjson_record_t* record, int indent, const char *buffer)
{
	int i;

	for (i = 0; i < indent; i++)
		fprintf(file, "    ");
	if (record->rectype == REXJSON_RECORD_BEGIN) {
		fprintf(file, "%7d [BEGIN] ", (int)record->valuesize);
	} else {
		fprintf(file, "%7d [End  ] ", (int)record->valuesize);
	}
	if (record->namesize) {
		fprintf(file, "(NAME : %.*s) ", (int)record->namesize, &buffer[record->name]);
	}
	switch (record->valtype) {
	case REXJSON_VALUE_NULL:
		fprintf(file, "NULL    : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_TRUE:
		fprintf(file, "TRUE    : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_FALSE:
		fprintf(file, "FALSE   : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_INT:
		fprintf(file, "INT     : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_NUMBER:
		fprintf(file, "NUMBER  : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_STRING:
		fprintf(file, "STRING  : %.*s\n", (int)record->valuesize, &buffer[record->value]);
		break;
	case REXJSON_VALUE_ARRAY:
		fprintf(file, "ARRAY   :\n");
		break;
	case REXJSON_VALUE_OBJECT:
		fprintf(file, "OBJECT  :\n");
		break;
	}
}

void rexjson_dump_records(FILE *file, rexjson_record_t* recs, size_t recsize, const char* buffer)
{
	size_t i = 0, indent = 0;

	while (i < recsize && recs[i].rectype) {
		if (recs[i].rectype == REXJSON_RECORD_BEGIN) {
			rexjson_record_dump(file, &recs[i], indent, buffer);
			indent++;
		} else {
			indent--;
			rexjson_record_dump(file, &recs[i], indent, buffer);
		}
		i++;
	}
}

ssize_t rexjson_parse_buffer(rexjson_t *ctx, rexjson_record_t* recs, size_t recsize, const char* buffer, size_t bufsize, size_t maxlevels)
{
	rexjson_init(ctx, recs, recsize, buffer, bufsize, maxlevels);
	rexjson_next_token(ctx);
	return rexjson_parse_value(ctx);
}

static ssize_t rexjson_recordtree_end(rexjson_record_t* recs, size_t recsize, ssize_t rec_index)
{
	size_t s = 0;
	rexjson_record_t* rec;

	if (rec_index < 0 || rec_index >= (ssize_t)recsize || recs[rec_index].rectype != REXJSON_RECORD_BEGIN)
		return -1;
	rec = &recs[rec_index];
	for (s = 0; rec_index < (ssize_t)recsize && recs[rec_index].rectype != REXJSON_RECORD_NONE; rec_index++) {
		rec = &recs[rec_index];
		if (rec->rectype == REXJSON_RECORD_BEGIN)
			++s;
		if (rec->rectype == REXJSON_RECORD_END)
			--s;
		if (s == 0)
			return rec_index;
	}
	return -1;
}

ssize_t rexjson_recordtree_firstchild(rexjson_record_t* recs, size_t recsize, ssize_t rec_index)
{
	if (rec_index < 0 || rec_index + 1 >= (ssize_t)recsize || recs[rec_index].rectype != REXJSON_RECORD_BEGIN)
		return -1;
	if (recs[rec_index + 1].rectype == REXJSON_RECORD_BEGIN)
		return rec_index + 1;
	return -1;
}

ssize_t rexjson_recordtree_next(rexjson_record_t* recs, size_t recsize, ssize_t rec_index)
{
	ssize_t end_index = rexjson_recordtree_end(recs, recsize, rec_index);

	if (end_index < 0)
		return -1;
	if (end_index + 1 < (ssize_t)recsize && recs[end_index + 1].rectype == REXJSON_RECORD_BEGIN)
		return end_index + 1;
	return -1;
}

void rexjson_dump_tree(FILE *file, rexjson_record_t* recs, size_t recsize, size_t record, const char* buffer, int indent)
{
	ssize_t child;
	rexjson_record_dump(file, &recs[record], indent, buffer);
	if ((child = rexjson_recordtree_firstchild(recs, recsize, record)) < 0)
		return;
	rexjson_dump_tree(file, recs, recsize, child, buffer, indent + 1);
	while ((child = rexjson_recordtree_next(recs, recsize, child)) >= 0) {
		rexjson_dump_tree(file, recs, recsize, child, buffer, indent + 1);
	}
}

#ifdef REXJSON_TEST_MAIN 
int main(int argc, const char *argv[])
{
	rexjson_t ctx;
	rexjson_record_t values[200];

	char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}}";
	char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[]="[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n		\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";

	if (rexjson_parse_buffer(&ctx, values, sizeof(values)/sizeof(values[0]), text1, sizeof(text1), 10) > 0) {
		rexjson_dump_tree(stdout, values, sizeof(values)/sizeof(values[0]), 0, text1, 0);
		fprintf(stdout, "\n");
	}
	if (rexjson_parse_buffer(&ctx, values, sizeof(values)/sizeof(values[0]), text2, sizeof(text2), 10) > 0) {
		rexjson_dump_tree(stdout, values, sizeof(values)/sizeof(values[0]), 0, text2, 0);
		fprintf(stdout, "\n");
	}
	if (rexjson_parse_buffer(&ctx, values, sizeof(values)/sizeof(values[0]), text3, sizeof(text3), 10) > 0) {
		rexjson_dump_tree(stdout, values, sizeof(values)/sizeof(values[0]), 0, text3, 0);
		fprintf(stdout, "\n");
	}
	if (rexjson_parse_buffer(&ctx, values, sizeof(values)/sizeof(values[0]), text4, sizeof(text4), 10) > 0) {
		rexjson_dump_tree(stdout, values, sizeof(values)/sizeof(values[0]), 0, text4, 0);
		fprintf(stdout, "\n");
	}
	if (rexjson_parse_buffer(&ctx, values, sizeof(values)/sizeof(values[0]), text5, sizeof(text5), 10) > 0) {
		rexjson_dump_tree(stdout, values, sizeof(values)/sizeof(values[0]), 0, text5, 0);
		fprintf(stdout, "\n");
	}
	return 0;
}
#endif
