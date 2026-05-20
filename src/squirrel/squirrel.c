#include "squirrel.h"
#include "interfaces/sys.h"

const char* get_context_name(ScriptContext context) {
  switch (context) {
  case SC_SERVER:
    return "SERVER";
  case SC_CLIENT:
    return "CLIENT";
  case SC_UI:
    return "UI";
  default:
    ns_logf(LOG_WARN, "%d is not a valid vm context", context);
    return "INVALID";
  }
}
