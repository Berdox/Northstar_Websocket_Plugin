#pragma once
#include <stdint.h>

#define PLUGIN_CONTEXT_DEDICATED 0x1

struct Color
{
	int r = 0;
	int g = 0;
	int b = 0;

	Color(int r, int g, int b) : r(r), g(g), b(b) {}

	inline int64_t bitflag() const
	{
		return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
	}
};

// this is the plugin name
#define PLUGIN_NAME "WS_Plugin"

// this is the name northstar uses when this plugin is logging to the console
#define PLUGIN_LOG_NAME "WS_Plugin"

// this is the color northstar will render the log name in
#define PLUGIN_COLOR Color(151, 41, 214 );

// this is the name of the squirrel constant northstar creates
#define PLUGIN_DEPENDENCY_NAME "WS_PLUGIN"

// this is a bitfield that determines if Northstar should load this plugin
#define PLUGIN_CONTEXT PLUGIN_CONTEXT_DEDICATED;