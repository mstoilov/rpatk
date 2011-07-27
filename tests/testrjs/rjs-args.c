#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rlib/rstring.h"
#include "rjs/rjs.h"


int main(int argc, char *argv[])
{
	rjs_engine_t *jse = rjs_engine_create();
	char test_script1[] = "print('ARGS[0]: ' + ARGS[0]); print('ARGS[1]: ' + ARGS[1]);print('ARGS.count: ' + ARGS.count);(ARGS[0]+ARGS[1])*3;";

//	jse->debugexec = 1;
	r_printf("result: %ld\n", rvm_reg_signed(rjs_engine_args_exec_s(jse, test_script1, 2, rvm_reg_create_signed(2), rvm_reg_create_signed(3))));
	rjs_engine_destroy(jse);
	return 0;
}
