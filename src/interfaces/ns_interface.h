#ifndef NS_INTERFACE_H
#define NS_INTERFACE_H

#include "interfaces/interface.h"
#include <windows.h>

typedef struct {
  HMODULE handle;
} NorthstarData;

void init_ns_interface(HMODULE ns_module, NorthstarData *init_data);
void *ns_create_interface(char *name, InterfaceStatus *status);

extern NorthstarData g_ns_data;
extern HMODULE g_ns_module;
#endif
