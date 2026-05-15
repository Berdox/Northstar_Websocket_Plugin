#include "../../../../include/interfaces/exposed/IPluginId/IPluginId.h"
#include "../../../../include/interfaces/interfaces.h"
#include "../../../../include/PluginConfiguration.h"

class CPluginId : public IPluginId
{
public:
	const char* GetString(PluginString prop)
	{
		switch (prop)
		{
		case PluginString::NAME:
			return PLUGIN_NAME;
		case PluginString::LOG_NAME:
			return PLUGIN_LOG_NAME;
		case PluginString::DEPENDENCY_NAME:
			return PLUGIN_DEPENDENCY_NAME;
		default:
			return 0;
		}
	}

	int64_t GetField(PluginField prop)
	{
		switch (prop)
		{
		case PluginField::CONTEXT:
			return PLUGIN_CONTEXT;
		case PluginField::COLOR:
		{
			Color c = PLUGIN_COLOR;
			return c.bitflag();
		}
		default:
			return 0;
		}
	}
};

EXPOSE_SINGLE_INTERFACE(CPluginId, IPluginId, PLUGIN_ID_VERSION);