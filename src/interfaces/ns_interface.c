#include "ns_interface.h"

NorthstarData g_ns_data = {0};
HMODULE g_ns_module = 0;

CreateInterfaceFn _p_ns_create_interface;

void init_ns_interface(HMODULE ns_module, NorthstarData *init_data) {
  _p_ns_create_interface =
      (CreateInterfaceFn)GetProcAddress(ns_module, "CreateInterface");
  g_ns_module = ns_module;

  g_ns_data.handle = init_data->handle;
}

void *ns_create_interface(char *name, InterfaceStatus *status) {
  return _p_ns_create_interface(name, status);
}
