#include "interfaces/cvar.h"
#include "interfaces/engine_interface.h"
#include "interfaces/sys.h"

ICvar *g_pCVar = 0;

void engine_cvar_interface_init() {
  g_pCVar = engine_create_interface(CVAR_INTERFACE_VERSION, 0);
  ns_log(LOG_INFO, "initialized cvar interface");
}
