#ifndef SQUIRREL_H
#define SQUIRREL_H

#include "bindings/assert.h"
#include <stdint.h>

typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef float SQFloat;
typedef char SQChar;
typedef SQUnsignedInteger SQBool;

typedef enum {
  SQRESULT_ERROR = -1,
  SQRESULT_NULL = 0,
  SQRESULT_NOTNULL = 1,
} SQRESULT;

typedef enum {
  SC_SERVER,
  SC_CLIENT,
  SC_UI,
} ScriptContext;

typedef enum {
  OT_USERPOINTER = 0x800,
  OT_VECTOR = 0x40000,
  OT_NULL = 0x1000001,
  OT_BOOL = 0x1000008,
  OT_INTEGER = 0x5000002,
  OT_FLOAT = 0x5000004,
  OT_STRING = 0x8000010,
  OT_ARRAY = 0x8000040,
  OT_CLOSURE = 0x8000100,
  OT_NATIVECLOSURE = 0x8000200,
  OT_ASSET = 0x8000400,
  OT_THREAD = 0x8001000,
  OT_FUNCPROTO = 0x8002000,
  OT_CLAAS = 0x8004000,
  OT_STRUCT = 0x8200000,
  OT_WEAKREF = 0x8010000,
  OT_TABLE = 0xA000020,
  OT_USERDATA = 0xA000080,
  OT_INSTANCE = 0xA008000,
  OT_ENTITY = 0xA400000,
} SQObjectType;

typedef union {
  void* userdata;
} SQObjectValue;

typedef struct {
  SQObjectType type;
  int structNumber;
  SQObjectValue value;
} SQObject;

typedef struct HSquirrelVM HSquirrelVM;
typedef HSquirrelVM* SQVM;

// too lazy to fill in the rest.
typedef struct {
  void* vftable;
  HSquirrelVM* sqvm;
  char gap_10[8];
  SQObject unkObj;
  int64_t unk;
  char gap_30[12];
  ScriptContext context;
  char gap_40[648];
  char* (*formatString)(int64_t self, const char* format, ...);
  char gap_2D0[24];
} CSquirrelVM;

ASSERT_SIZE(CSquirrelVM, 744);
ASSERT_OFFSET(CSquirrelVM, context, 0x3C);

const char* get_context_name(ScriptContext context);

#endif
