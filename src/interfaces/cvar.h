#ifndef IFACE_CVAR_H
#define IFACE_CVAR_H
#include "cvar/cvar.h"

#define CVAR_INTERFACE_VERSION "VEngineCvar007"

extern ICvar *g_pCVar;

void engine_cvar_interface_init();

#endif
