#ifndef SQVM_H
#define SQVM_H

#include "squirrel/squirrel.h"

// way too lazy to finish this struct ngl
typedef struct HSquirrelVM {
  void* vftable;
  int uiRef;

} HSquirrelVM;

#endif
