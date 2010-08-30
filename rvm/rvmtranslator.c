#include "rmem.h"
#include "rvmtranslator.h"


rvm_translator_t *rvm_translator_create()
{
	rvm_translator_t *trans;

	trans = (rvm_translator_t *)r_malloc(sizeof(*trans));
	if (!trans)
		return (NULL);
	r_memset(trans, 0, sizeof(*trans));
	return trans;
}


void rvm_translator_destroy(rvm_translator_t *trans)
{

	r_free(trans);
}
