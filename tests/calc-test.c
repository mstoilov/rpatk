#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpadbex.h"


#define LC_STACK_SIZE 1024


typedef struct lc_stack_s {
	int buf[LC_STACK_SIZE];
	unsigned int pos;
} lc_stack_t;

void lc_stack_push(lc_stack_t *stack, int n)
{
	if (stack->pos < LC_STACK_SIZE) {
		stack->pos += 1;
		stack->buf[stack->pos] = n;
	}
}

int lc_stack_pop(lc_stack_t *stack)
{
	int n = 0;
	if (stack->pos > 0) {
		n = stack->buf[stack->pos];
		stack->pos -= 1;
	}
	return n;
}


int lc_print(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
#ifdef DEBUGPRINT
	fprintf(stdout, "%s: ", name);
	fwrite(input, sizeof(char), size, stdout);
	fprintf(stdout, "\n");
#endif
	return size;
}


int lc_addop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	lc_stack_t *stack = (lc_stack_t*)userdata;
	int op1 = 0, op2 = 0;

	op2 = lc_stack_pop(stack);
	op1 = lc_stack_pop(stack);	
	lc_stack_push(stack, op1 + op2);
	lc_print(name, userdata, input, size, reason, start, end);
	return size;
}

int lc_subop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	lc_stack_t *stack = (lc_stack_t*)userdata;
	int op1 = 0, op2 = 0;

	op2 = lc_stack_pop(stack);
	op1 = lc_stack_pop(stack);	
	lc_stack_push(stack, op1 - op2);
	lc_print(name, userdata, input, size, reason, start, end);
	return size;
}


int lc_divop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	lc_stack_t *stack = (lc_stack_t*)userdata;
	int op1 = 0, op2 = 0;

	op2 = lc_stack_pop(stack);
	op1 = lc_stack_pop(stack);	
	lc_stack_push(stack, op1 / op2);
	lc_print(name, userdata, input, size, reason, start, end);
	return size;
}

int lc_mulop_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	lc_stack_t *stack = (lc_stack_t*)userdata;
	int op1 = 0, op2 = 0;

	op2 = lc_stack_pop(stack);
	op1 = lc_stack_pop(stack);	
	lc_stack_push(stack, op1 * op2);
	lc_print(name, userdata, input, size, reason, start, end);
	return size;
}

int lc_integer_callback(const char *name, void *userdata, const char *input, unsigned int size, unsigned int reason, const char *start, const char *end)
{
	lc_stack_t *stack = (lc_stack_t*)userdata;

	lc_stack_push(stack, atoi(input));
	lc_print(name, userdata, input, size, reason, start, end);
	return size;
}

int main(int argc, char *argv[])
{
	rpa_dbex_handle hDbex;
	lc_stack_t stack;
	int res;

	memset(&stack, 0, sizeof(stack));
	hDbex = rpa_dbex_create();
	/*
	 * Important: Callbacks must be specified before the rules!
	 */
	rpa_dbex_add_callback(hDbex, "mulop", RPA_REASON_MATCHED, lc_mulop_callback, &stack);
	rpa_dbex_add_callback(hDbex, "divop", RPA_REASON_MATCHED, lc_divop_callback, &stack);
	rpa_dbex_add_callback(hDbex, "addop", RPA_REASON_MATCHED, lc_addop_callback, &stack);
	rpa_dbex_add_callback(hDbex, "subop", RPA_REASON_MATCHED, lc_subop_callback, &stack);
	rpa_dbex_add_callback(hDbex, "integer", RPA_REASON_MATCHED, lc_integer_callback, &stack);

	rpa_dbex_open(hDbex);
	res = rpa_dbex_load_string(hDbex, "S		::= [#x20]+");
	res = rpa_dbex_load_string(hDbex, "digit	::= [0-9]");
	res = rpa_dbex_load_string(hDbex, "integer	::= <:digit:>+");
	res = rpa_dbex_load_string(hDbex, "term		::= <:integer:> | '('<:S:>? <:expr:> <:S:>? ')'");
	res = rpa_dbex_load_string(hDbex, "mulop	::= <:mulex:> <:S:>? '*' <:S:>? <:term:>");
	res = rpa_dbex_load_string(hDbex, "divop	::= <:mulex:> <:S:>? '/' <:S:>? <:term:>");
	res = rpa_dbex_load_string(hDbex, "mulex	::= <:mulop:> | <:divop:> | <:term:>");
	res = rpa_dbex_load_string(hDbex, "addop	::= <:expr:> <:S:>? '+' <:S:>? <:mulex:>");
	res = rpa_dbex_load_string(hDbex, "subop	::= <:expr:> <:S:>? '-' <:S:>? <:mulex:>");
	res = rpa_dbex_load_string(hDbex, "expr		::= <:addop:> | <:subop:> | <:mulex:>");
	res = rpa_dbex_load_string(hDbex, "exec		::= <:expr:>");
	rpa_dbex_close(hDbex);
	if (argc > 1) {
		res = rpa_dbex_parse(hDbex, rpa_dbex_default_pattern(hDbex), argv[1], argv[1], argv[1] + strlen(argv[1]));
	}
	rpa_dbex_destroy(hDbex);

	fprintf(stdout, "%d\n", lc_stack_pop(&stack));
	return 0;
}
