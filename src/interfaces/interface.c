#include "interface.h"
#include "interfaces/plugin_callbacks.h"
#include <stdlib.h>
#include <string.h>

#define CALLBACK_INTERFACE_VERSION "PluginCallbacks001"

InterfaceReg *g_interfaces = 0;

void interface_create_reg(const char *name, void *(*createFn)()) {
  InterfaceReg *new = malloc(sizeof(InterfaceReg));
  new->m_CreateFn = createFn;
  new->name = name;
  new->next = g_interfaces;
  g_interfaces = new;
}

void *CreateInterface(const char *name, InterfaceStatus *status) {
  InterfaceReg *reg;

  for (reg = g_interfaces; reg; reg = reg->next) {
    if (strcmp(name, reg->name) == 0) {
      if (status)
        *status = IFACE_OK;
      return reg->m_CreateFn();
    }
  }

  if (status)
    *status = IFACE_FAILED;
  return 0;
}

void interfaces_init() {
  interface_create_reg(CALLBACK_INTERFACE_VERSION, CreatePluginCallbacks);
  interface_create_reg(PLUGIN_ID_VERSION, CreatePluginId);
}

void interfaces_destroy() {
  for (InterfaceReg *reg = g_interfaces; reg; reg = reg->next) {
    free(reg);
  }
}
