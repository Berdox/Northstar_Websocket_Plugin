#pragma once

#include <stdint.h>

#define PLUGIN_ID_VERSION "PluginId001"

enum class PluginString : int
{
	NAME = 0,
	LOG_NAME = 1,
	DEPENDENCY_NAME = 2,
};

enum class PluginField : int
{
	CONTEXT = 0,
	COLOR = 1,
};

class IPluginId
{
public:
	virtual const char* GetString(PluginString prop) = 0;
	virtual int64_t GetField(PluginField prop) = 0;
};