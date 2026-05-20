#ifndef _BINDING_MODULE_H
#define _BINDING_MODULE_H

#include <minwindef.h>
#include <stdint.h>

extern HMODULE g_server;
extern HMODULE g_client;
extern HMODULE g_engine;

static inline void *ptr_at(void *mod, uint64_t offset) {
  return (void *)((uint64_t)mod + offset);
};

#endif
