#ifndef SYS_H
#define SYS_H

#include <windows.h>

#define NS_SYS_VERSION "NSSys001"

typedef enum {
  LOG_INFO,
  LOG_WARN,
  LOG_ERR,
} LogLevel;

typedef struct CSys {
  struct {
    void (*log
    )(struct CSys *self, HMODULE handle, LogLevel level, const char *msg);
    void (*unload)(struct CSys *self, HMODULE handle);
    void (*reload)(struct CSys *self, HMODULE handle);
  } *vftable;
} CSys;

void sys_init();

void ns_log(LogLevel level, const char *msg);
void ns_logf(LogLevel level, const char *format, ...);
void ns_unload_self();

#endif
