#include "rvmrelocmap.h"
#include "rmem.h"

rvm_relocmap_t *rvm_relocmap_create()
{
	rvm_relocmap_t *relocmap;

	relocmap = (rvm_relocmap_t*)r_zmalloc(sizeof(*relocmap));
	if (!relocmap)
		return NULL;
	relocmap->records = r_array_create(sizeof(rvm_relocrecord_t));
	return relocmap;
}


void rvm_relocmap_destroy(rvm_relocmap_t *relocmap)
{
	r_object_destroy((robject_t*)relocmap->records);
	r_free(relocmap);
}


void rvm_relocmap_clear(rvm_relocmap_t *relocmap)
{
	r_array_setlength(relocmap->records, 0);
}


rlong rvm_relocmap_add(rvm_relocmap_t *relocmap, rvm_reloctype_t type, rulong offset, rulong label)
{
	rvm_relocrecord_t record;
	record.type = type;
	record.offset = offset;
	record.label = label;

	return r_array_add(relocmap->records, &record);
}


rvm_relocrecord_t *rvm_relocmap_get(rvm_relocmap_t *relocmap, rulong index)
{
	if (index >= r_array_length(relocmap->records))
		return NULL;
	return (rvm_relocrecord_t *)r_array_slot(relocmap->records, index);
}


rulong rvm_relocmap_length(rvm_relocmap_t *relocmap)
{
	return r_array_length(relocmap->records);
}


rint rvm_relocmap_relocate(rvm_relocmap_t *relocmap, rvm_codemap_t *codemap, rvm_asmins_t *code, rvm_codelabel_t **err)
{
	rlong index;
	rvm_relocrecord_t *reloc;
	rword value;

	for (index = 0; index < r_array_length(relocmap->records); index++) {
		reloc = rvm_relocmap_get(relocmap, index);
		value = rvm_codemap_resolve(codemap, reloc->label, err);
		if (value == (rword)-1)
			return -1;
		if (reloc->type == RVM_RELOC_BRANCH) {
			code[reloc->offset].data.v.w = RVM_BYTE2CODE_OFFSET(value - (rword)&code[reloc->offset]);
		} else if (reloc->type == RVM_RELOC_JUMP) {
			code[reloc->offset].data.v.w = value - RVM_CODE2BYTE_OFFSET(1);
		} else {
			code[reloc->offset].data.v.w = value;
		}
	}
	return 0;
}
