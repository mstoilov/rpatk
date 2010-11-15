#include "rpamatch.h"
#include "rpastring.h"
#include "rpamem.h"


static unsigned int rpa_match_special_getid(rpa_class_t *cls)
{
	return (RPA_MATCH_SPECIAL_CLASSID | RPA_MATCH_CLASSID);
}


static const char *rpa_match_special_getstr(rpa_class_t *cls)
{
	return "matchspecial";
}


static int rpa_match_special_dump(rpa_class_t *cls, char *buffer, unsigned int size)
{
	return 0;
}


static void rpa_match_special_destroy(rpa_class_t *cls)
{
	rpa_match_t *match = (rpa_match_t*)cls;
	rpa_match_cleanup(match);
	rpa_free(match);
}


static rpa_class_methods_t rpa_match_special_methods = {
	rpa_match_special_getid,
	rpa_match_special_getstr,
	rpa_match_special_dump,
	rpa_match_special_destroy,
};


rpa_match_t * rpa_match_special_create_namesize(
	const char *name,
	unsigned int namesiz,
	rpa_matchfunc_t match_function_id)
{
	rpa_match_t *newMatch;
	
	newMatch = (rpa_match_t *)rpa_malloc(sizeof(*newMatch));
	if (!newMatch)
		return ((void*)0);
	rpa_memset(newMatch, 0, sizeof(*newMatch));
	return rpa_match_init(newMatch, name, namesiz, match_function_id, &rpa_match_special_methods);
}


rpa_match_t *rpa_match_special_create(
	const char *name,
	rpa_matchfunc_t match_function_id)
{
	return rpa_match_special_create_namesize(name, rpa_strlen(name), match_function_id);
}
