#include <stdio.h>

#include "rlib/robject.h"

typedef struct _RTestStruct {
	int i;
	char a;
	char b;
} RTestStruct;

int main(int argc, char *argv[])
{
	RTestStruct t;

	fprintf(stdout, 
			"It works. Major: %d, Minor: %d, sizeof(t) = %lu\n", 
			r_object_major_version(), 
			r_object_minor_version(),
			sizeof(t));
	return 0;
}
