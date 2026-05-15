#include "../../include/interfaces/Northstar.h"
#include <stdio.h>

bool InitNSSys(HMODULE nsModule) {
	g_ns_CreateInterface = (CreateInterface_T)GetProcAddress(nsModule, "CreateInterface");
	if (!g_ns_CreateInterface) return false;

	g_nssys = (INSSys*)g_ns_CreateInterface("NSSys001", 0);
	return g_nssys != 0;
}