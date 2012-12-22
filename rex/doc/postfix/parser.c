#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lookahead;
char buffer[256];

void expr();
void term();
void rest();

void match(int c)
{
	if (lookahead != c) {
		fprintf(stdout, "\nSyntax Error!\n");
		exit(1);
	}
again:
	lookahead = get_token(buffer, sizeof(buffer)-1);
	if (lookahead == TOKEN_SPACE)
		goto again;
}

void term()
{
	if (lookahead == TOKEN_TERM) {
		fprintf(stdout, "%s ", buffer);
		match(lookahead);
	}
}

void expr()
{
	term();
	rest();
}

void rest()
{
	if (lookahead == '+') {
		match('+');
		term();
		fprintf(stdout, "+ ");
		rest();
	} else if (lookahead == '-') {
		match('-');
		term();
		fprintf(stdout, "- ");
		rest();
	} else {
		/*
		 * Epsilon case: do nothing
		 */
	}
}

int main(int argc, char *argv[])
{
	lookahead = get_token(buffer, sizeof(buffer));
	expr();
	fprintf(stdout, "\n");
	return 0;
}
