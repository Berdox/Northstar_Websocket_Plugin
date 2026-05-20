#include "engine_interface.h"

HMODULE g_engine_module;

CreateInterfaceFn _p_g_engine_create_interface;

void init_engine_interface(HMODULE engine_module) {
  _p_g_engine_create_interface =
      (CreateInterfaceFn)GetProcAddress(engine_module, "CreateInterface");
}

void *engine_create_interface(char *name, InterfaceStatus *status) {
  return _p_g_engine_create_interface(name, status);
}
