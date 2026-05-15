#include "./WebsocketPlugin.h"
#include "./interfaces/Northstar.h"
#include <stdio.h>

void InitWSPlugin(HMODULE pluginHandle, HMODULE nsHandle) {
	g_handle = pluginHandle;

	if (!InitNSSys(nsHandle)) {
		printf("Could not initialize NSSys");
		return;
	}

	g_nssys->Log(pluginHandle, LogLevel::INFO, "HelloWorld");
}