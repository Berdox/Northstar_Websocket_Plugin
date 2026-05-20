#ifndef ENGINE_IFACE_H
#define ENGINE_IFACE_H

#include "interfaces/interface.h"
#include <windows.h>

extern HMODULE g_engine_module;

void init_engine_interface(HMODULE engine_module);
void *engine_create_interface(char *name, InterfaceStatus *status);

#endif
