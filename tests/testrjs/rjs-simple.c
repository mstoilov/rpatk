#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rstring.h"
#include "rjs.h"


int main(int argc, char *argv[])
{
	rjs_engine_t *jse = rjs_engine_create();
	char test_script1[] = "(2+3)*3;";
	char test_script2[] = "function person(name, age, job) {this.name = name; this.age = age; this.job = job;} var me = new person('Martin', 25, 'Slacker');";

//	jse->debugexec = 1;
	r_printf("script: %s result: %ld\n", test_script1, rvm_reg_long(rjs_engine_exec_s(jse, test_script1)));
	rjs_engine_exec_s(jse, test_script2);
	r_printf("Name: %s\n", r_string_ansi(rvm_reg_string(rjs_engine_exec_s(jse, "me.name;"))));
	r_printf("Job: %s\n", r_string_ansi(rvm_reg_string(rjs_engine_exec_s(jse, "me.job;"))));
	r_printf("Description: %s\n", r_string_ansi(rvm_reg_string(rjs_engine_exec_s(jse, "'Name = ' + me.name + ', Job = ' + me.job;"))));
	rjs_engine_destroy(jse);
	return 0;
}
