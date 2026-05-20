#include "cvar.h"
#include "bindings/module.h"

void (*ConVarMalloc)(void* pConVarMalloc, int a2, int a3) = 0;
void (*ConVarRegister)(ConVar* pConVar, const char* pszName, const char* pszDefaultValue, int nFlags, const char* pszHelpString, bool bMin, float fMin, bool bMax, float fMax, void* pCallback);

void engine_cvar_init() {
  ConVarMalloc = ptr_at(g_engine, 0x415C20);
  ConVarRegister = ptr_at(g_engine, 0x417230);
}
