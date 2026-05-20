#include "interfaces/sys.h"
#include "interfaces/ns_interface.h"
#include <stdio.h>

CSys *sys;

void sys_init() { sys = ns_create_interface(NS_SYS_VERSION, 0); }

void ns_log(LogLevel level, const char *msg) {
  if (!sys)
    return;
  sys->vftable->log(sys, g_ns_data.handle, level, msg);
}

void ns_logf(LogLevel level, const char *format, ...) {
  char buf[128] = {0};
  va_list args;
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  buf[127] = 0; // todo dies das
  ns_log(level, buf);
}

void ns_unload_self() { sys->vftable->unload(sys, g_ns_data.handle); }
