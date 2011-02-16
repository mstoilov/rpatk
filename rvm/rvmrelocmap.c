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
}


void rvm_relocmap_clear(rvm_relocmap_t *relocmap)
{
	r_array_setlength(relocmap->records, 0);
}


rvm_relocrecord_t *rvm_relocmap_add(rvm_relocmap_t *relocmap, rulong offset, rulong label)
{
	rvm_relocrecord_t record;
	rint index;
	record.offset = offset;
	record.label = label;

	index = r_array_add(relocmap->records, &record);
	return (rvm_relocrecord_t *)r_array_slot(relocmap->records, index);
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


rlong rvm_relocmap_relocate(rvm_relocmap_t *relocmap, rvm_codemap_t *codemap, rvm_asmins_t *code)
{
	rlong index;
	rvm_codelabel_t *label;
	rvm_relocrecord_t *reloc;

	for (index = 0; index < r_array_length(relocmap->records); index++) {
		reloc = rvm_relocmap_get(relocmap, index);
		label = rvm_codemap_label(codemap, reloc->label);
		if (label == NULL || label->type == RVM_CODELABEL_INVALID)
			return index;
		code[reloc->offset].data.v.u64 = (ruint64)(code + label->loc.index);
	}
	return -1;
}
