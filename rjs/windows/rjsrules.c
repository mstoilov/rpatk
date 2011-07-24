#include <windows.h>
#include "resource.h"
#include "rjs/rjsrules.h"

static const char *rules = NULL;
static DWORD rules_size = 0;


static void LoadRulesFromResource(int name, int type, DWORD *size, const char** data)
{
	HMODULE handle = GetModuleHandle(NULL);
	HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));
	HGLOBAL rcData = LoadResource(handle, rc);
	*size = SizeofResource(handle, rc);
	*data = (const char*)LockResource(rcData);
}


const char *rjs_rules_get()
{
	if (!rules)
		LoadRulesFromResource(IDR_ECMA262, TEXTFILE, &rules_size, &rules);
	return rules;
}


rsize_t rjs_rules_size()
{
	if (!rules)
		LoadRulesFromResource(IDR_ECMA262, TEXTFILE, &rules_size, &rules);
	return rules_size;
}
