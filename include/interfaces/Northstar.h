#pragma once

#include "./Northstar/INSSys.h"

typedef void* (*CreateInterface_T)(const char*, int*);
static CreateInterface_T g_ns_CreateInterface;

inline INSSys* g_nssys;

bool InitNSSys(HMODULE nsHandle);