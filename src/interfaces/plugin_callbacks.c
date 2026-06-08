#include "plugin_callbacks.h"
#include "bindings/module.h"
#include "bindings/native_closures.h"
#include "bindings/relay.h"
#include "cvar/cvar.h"
#include "interfaces/cvar.h"
#include "interfaces/engine_interface.h"
#include "interfaces/sys.h"

#define PLUGIN_NAME " EXAMPLE "
#define PLUGIN_LOG_NAME                                                                                                \
  PLUGIN_NAME // all northstar log tags use 9 characters. Keep the same length to make your logs look a bit nicer
#define PLUGIN_DEPENDENCY_NAME PLUGIN_NAME
#define PLUGIN_CONTEXT (PCTX_DEDICATED | PCTX_CLIENT)

void sv_initialize(HMODULE sv) {
  ns_log(LOG_INFO, "initializing server bridge");
  g_server = sv;
  init_relay_sv(sv);
}

void cl_initialize(HMODULE cl) {
  ns_log(LOG_INFO, "initializing client bridge");
  g_client = cl;
  init_relay_cl(cl);
}

void en_initialize(HMODULE en) {
  ns_log(LOG_INFO, "initializing engine bridge");
  g_engine = en;
  init_engine_interface(en);
  engine_cvar_interface_init();
}

// After the plugin has been reloaded, make sure to redefine everything necessary
void reinitialize() {
  HMODULE sv_module = GetModuleHandleA("server.dll");
  if (sv_module)
    sv_initialize(sv_module);

  HMODULE cl_module = GetModuleHandleA("client.dll");
  if (cl_module)
    cl_initialize(cl_module);

  HMODULE en_module = GetModuleHandleA("engine.dll");
  if (en_module)
    en_initialize(en_module);
}

const char* GetString(IPluginId* self, PluginString prop) {
  switch (prop) {
  case ID_NAME:
    return PLUGIN_NAME;
    break;
  case ID_LOG_NAME:
    return PLUGIN_LOG_NAME;
    break;
  case ID_DEPENDENCY_NAME:
    return PLUGIN_DEPENDENCY_NAME;
    break;
  default:
    return 0;
  }
}

int64_t GetField(IPluginId* self, PluginField prop) {
  switch (prop) {
  case ID_CONTEXT:
    return PLUGIN_CONTEXT;
  default:
    return 0;
  }
}

void Init(IPluginCallbacks* self, HMODULE module, NorthstarData* data, char reloaded) {
  init_ns_interface(module, data);
  sys_init();

  ns_log(LOG_INFO, "initializing");

  if (reloaded)
    reinitialize();

  ns_log(LOG_INFO, "Successfully initialized");
}

void Finalize(IPluginCallbacks* self) {
  /*
   * Get interfaces from other plugins you depend on here like this
   *
   * HMODULE my_dependency_module = GetModuleHandleA("MyDependency.dll");
   * if(my_dependency_module)
   * {
   * 	void* my_dependency_interface = GetProcAddress(my_dependency_module,
   * "CreateInterface");
   * 	...
   * }
   */

  ns_log(LOG_INFO, "Finalized.");
}

bool Unload(IPluginCallbacks* self) {
  ns_log(LOG_INFO, "Unloading.");

  /*
   * If this returns `false`, the plugin *will not be unloaded*.
   * It might be a good idea to never unload the plugin in builts meant to be distributed to increase stability since
   * plugins are (as of right now) only meant to be reloaded / unloaded at runtime for development purposes.
   * (This might change)
   *
   * #if DEV
   * return true;
   * #else
   * return false;
   * #endif
   */

  return true;
}

void OnSqvmCreated(IPluginCallbacks* self, CSquirrelVM* c_sqvm) {
  ns_logf(LOG_INFO, "created %s sqvm", get_context_name(c_sqvm->context));

  register_native_closures(c_sqvm);

  // sqapi(c_sqvm->context)->c_sq_define_constant(c_sqvm, "TEST_CONSTANT", 2);
}

void OnSqvmDestroyed(IPluginCallbacks* self, CSquirrelVM* c_sqvm) {
  ns_logf(LOG_INFO, "destroying %s sqvm", get_context_name(c_sqvm->context));

  deregister_native_closures(c_sqvm->context);
}

void OnLibraryLoaded(IPluginCallbacks* self, HMODULE module, const char* name) {
  ns_logf(LOG_INFO, "loaded library %s", name);

  if (strcmp(name, "server.dll") == 0) {
    sv_initialize(module);
    return;
  }

  if (strcmp(name, "client.dll") == 0) {
    cl_initialize(module);
    return;
  }

  if (strcmp(name, "engine.dll") == 0) {
    en_initialize(module);
    return;
  }
}

void RunFrame(IPluginCallbacks* self) {}

IPluginCallbacks g_pluginCallbacks = {
    .vftable = &(struct IPluginCallbacks_vftable
    ){.Init = Init,
      .Finalize = Finalize,
      .Unload = Unload,
      .OnSqvmCreated = OnSqvmCreated,
      .OnSqvmDestroyed = OnSqvmDestroyed,
      .OnLibraryLoaded = OnLibraryLoaded,
      .RunFrame = RunFrame}};

void* CreatePluginCallbacks() { return &g_pluginCallbacks; }

IPluginId g_pluginId = {.vftable = &(struct IPluginId_vftable){.GetString = GetString, .GetField = GetField}};

void* CreatePluginId() { return &g_pluginId; }
