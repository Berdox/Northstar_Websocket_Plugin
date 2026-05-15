#pragma once

#include "./interfaces/interfaces.h"
#include <windows.h>

inline HMODULE g_handle;

void InitWSPlugin(HMODULE pluginHandle, HMODULE nsHandle);