#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include "rtypes.h"
#include "rlib/rmem.h"
#include "rlib/rstring.h"
#include "rjs/rjsrules.h"



extern ruint32 _mh_execute_header[];
static const char *rjs_rules = NULL;
static size_t rjs_size = 0;

void rjs_rules_init(const char *segname, const char *sectname)
{
	int i, j;


	if (((struct mach_header*)_mh_execute_header)->magic == MH_MAGIC_64) {
		struct mach_header_64 *mheader = (void *)_mh_execute_header;
		const struct load_command *command = NULL;
		const struct segment_command_64 *segment = NULL;
		const struct section_64 *section = NULL;

		command = (void *)(mheader + 1);
		for (i = 0; i < mheader->ncmds; i++) {
			if (command->cmd != LC_SEGMENT_64) {
				command = (void *)(((char *)command) + command->cmdsize);
				continue;
			}
			segment = (void *)command;
			if (r_strncmp(segment->segname, segname, 16) != 0) {
				command = (void *)(((char *)command) + command->cmdsize);
				continue;
			}
			/* Found a segment named "rpa". */
			section = (void *)(segment + 1);
			for (j = 0; j < segment->nsects; j++, section++) {
				if (r_strncmp(section->sectname, sectname, 16) != 0) {
					continue;
				}
				rjs_rules = (void *)(section->addr);
				rjs_size = section->size;
				return;
			}
		}
	} else if (((struct mach_header*)_mh_execute_header)->magic == MH_MAGIC) {
		struct mach_header *mheader = (void *)_mh_execute_header;
		const struct load_command *command = NULL;
		const struct segment_command *segment = NULL;
		const struct section *section = NULL;

		command = (void *)(mheader + 1);
		for (i = 0; i < mheader->ncmds; i++) {
			if (command->cmd != LC_SEGMENT) {
				command = (void *)(((char *)command) + command->cmdsize);
				continue;
			}
			segment = (void *)command;
			if (r_strncmp(segment->segname, segname, 16) != 0) {
				command = (void *)(((char *)command) + command->cmdsize);
				continue;
			}
			/* Found a segment named "rpa". */
			section = (void *)(segment + 1);
			for (j = 0; j < segment->nsects; j++, section++) {
				if (r_strncmp(section->sectname, sectname, 16) != 0) {
					continue;
				}
				rjs_rules = (void *)((unsigned long)(section->addr));
				rjs_size = section->size;
				return;
			}
		}
	}
}


const char *rjs_rules_get()
{
	if (!rjs_rules) {
		rjs_rules_init("rpa", "ecma262");
	}
	return rjs_rules;
}


unsigned long rjs_rules_size()
{
	if (!rjs_rules) {
		rjs_rules_init("rpa", "ecma262");
	}
	return (unsigned long)rjs_size;
}
