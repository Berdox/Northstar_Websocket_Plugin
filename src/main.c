#include "bindings/native_closures.h"
#include "interfaces/interface.h"
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID reserved) {
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    interfaces_init(); // initialize all interfaces. You can make this static
                       // but it's easier to work with on the heap
    break;
  case DLL_PROCESS_DETACH:
    if (reserved)
      break;                      // don't cleanup if terminated
    interfaces_destroy();         // free all interfaces
  }

  return TRUE;
}
