#include "rvmnamedarray.h"
#include "rstring.h"
#include "rmem.h"


static void r_refstub_destroy(rref_t *ref)
{
	rvm_namedarray_destroy((rvm_namedarray_t*)ref);
}


static rref_t *r_refstub_copy(const rref_t *ptr)
{
	return (rref_t*) r_namedarray_copy((const rvm_namedarray_t*)ptr);
}


rvm_namedarray_t *rvm_namedarray_create()
{
	rvm_namedarray_t *namedarray;

	namedarray = (rvm_namedarray_t*)r_malloc(sizeof(*namedarray));
	if (!namedarray)
		return NULL;
	r_memset(namedarray, 0, sizeof(*namedarray));
	namedarray->members = r_array_create(sizeof(rvm_namedmember_t), NULL, NULL);
	namedarray->hash = r_hash_create(5, r_hash_strnequal, r_hash_strnhash);
	r_ref_init(&namedarray->ref, 1, RREF_TYPE_NONE, r_refstub_destroy, r_refstub_copy);
	return namedarray;
}


rvm_namedarray_t *r_namedarray_copy(const rvm_namedarray_t *src)
{
	int i;
	rvm_namedmember_t *m;
	rvm_namedarray_t *dst = rvm_namedarray_create();

	if (!dst)
		return NULL;
	for (i = 0; i < src->members->len; i++) {
		m = (rvm_namedmember_t *)r_array_slot(src->members, i);
		rvm_namedarray_add(dst, m->name->str, m->name->size, &m->val);
	}
	return dst;
}


void rvm_namedarray_destroy(rvm_namedarray_t *namedarray)
{
	int i;
	rvm_namedmember_t *member;
	int len = namedarray->members->len;

	for (i = 0; i < len; i++) {
		member = (rvm_namedmember_t *)r_array_slot(namedarray->members, i);
		r_free(member->name);
		RVM_REG_UNREF(&member->val);
	}
	r_array_destroy(namedarray->members);
	r_hash_destroy(namedarray->hash);
	r_free(namedarray);
}


rint rvm_namedarray_add(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize, const rvm_reg_t *pval)
{
	rvm_namedmember_t member;
	rint index;

	r_memset(&member, 0, sizeof(member));
	member.name = r_rstrdup(name, namesize);
	if (pval)
		member.val = *pval;
	index = r_array_add(namedarray->members, &member);
	r_hash_insert_indexval(namedarray->hash, (rconstpointer)member.name, index);
	return index;
}


rint rvm_namedarray_stradd(rvm_namedarray_t *namedarray, const rchar *name, const rvm_reg_t *pval)
{
	return rvm_namedarray_add(namedarray, name, r_strlen(name), pval);
}


rint rvm_namedarray_set(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize, const rvm_reg_t *pval)
{
	rvm_namedmember_t *member;
	rlong index = rvm_namedarray_lookup(namedarray, name, namesize);
	if (index < 0)
		return -1;
	member = (rvm_namedmember_t*)r_array_slot(namedarray->members, index);
	RVM_REG_UNREF(&member->val);
	if (pval)
		member->val = *pval;
	else
		RVM_REG_CLEAR(&member->val);
	if (rvm_reg_flagtst(&member->val, RVM_INFOBIT_REFOBJECT)) {
		if ((member->val.v.p = r_ref_copy((rref_t*)member->val.v.p)) == NULL)
			RVM_REG_CLEAR(&member->val);
	}
	return 0;
}


rint rvm_namedarray_strset(rvm_namedarray_t *namedarray, const rchar *name, const rvm_reg_t *pval)
{
	return rvm_namedarray_set(namedarray, name, r_strlen(name), pval);
}


rlong rvm_namedarray_lookup(rvm_namedarray_t *namedarray, const rchar *name, ruint namesize)
{
	rulong found;

	rstr_t lookupstr = {(char*)name, namesize};
	found = r_hash_lookup_indexval(namedarray->hash, &lookupstr);
	if (found == R_HASH_INVALID_INDEXVAL)
		return -1;
	return (rlong)found;
}



rlong rvm_namedarray_strlookup(rvm_namedarray_t *namedarray, const rchar *name)
{
	return rvm_namedarray_lookup(namedarray, name, r_strlen(name));
}


