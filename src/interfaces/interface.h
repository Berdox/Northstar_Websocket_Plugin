#ifndef INTERFACE_H
#define INTERFACE_H

typedef enum {
  IFACE_OK = 0,
  IFACE_FAILED,
} InterfaceStatus;

typedef struct InterfaceReg {
  void *(*m_CreateFn)();
  const char *name;
  struct InterfaceReg *next;
} InterfaceReg;

typedef void *(*CreateInterfaceFn)(char *name, InterfaceStatus *status);

void interfaces_init();

void interfaces_destroy();

// this has to be exposed for the launcher and other plugins that depend on this
// plugin's interfaces
void *CreateInterface(const char *name, InterfaceStatus *status);

#endif
