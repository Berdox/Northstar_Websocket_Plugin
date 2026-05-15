#include "../../../../include/interfaces/exposed/IPluginCallbacks/IPluginCallbacks.h"
#include "../../../../include/interfaces/interfaces.h"
#include "../../../../include/interfaces/Northstar.h"
#include "../../../../include/WS_Plugin.h"
#include <string>

class CPluginCallbacks : public IPluginCallbacks
{
	void Init(HMODULE northstarModule, const PluginNorthstarData* initData, bool reloaded) {
		InitWSPlugin(initData->pluginHandle, northstarModule);
	}

	void OnLibraryLoaded(HMODULE module, const char* name) {
		if (strcmp(name, "server.dll") == 0)
		{
			// initialize your server squirrel api here ...
		}

		if (strcmp(name, "client.dll") == 0)
		{
			// initialize your client squirrel api here ...
		}
	}

	void Finalize() {}

	void Unload() {}

	void OnSqvmCreated(void* c_sqvm) {}

	void OnSqvmDestroying(void* c_sqvm) {}

	void RunFrame() {}
};

EXPOSE_SINGLE_INTERFACE(CPluginCallbacks, IPluginCallbacks, PLUGIN_CALLBACKS_VERSION)