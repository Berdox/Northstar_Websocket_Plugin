#ifndef NATIVE_CLOSURES_H
#define NATIVE_CLOSURES_H

#include "squirrel/squirrel.h"

typedef enum SQNativeClosureReturnType {
  SQ_RET_FLOAT = 0x1,
  SQ_RET_VEC = 0x3,
  SQ_RET_INT = 0x5,
  SQ_RET_BOOL = 0x6,
  SQ_RET_ENT = 0xD,
  SQ_RET_STR = 0x21,
  SQ_RET_DEFAULT = 0x20,
  SQ_RET_ARR = 0x25,
  SQ_RET_ASSET = 0x28,
  SQ_RET_TABLE = 0x26,
} SQNativeClosureReturnType;

typedef SQRESULT (*SQNativeClosureImplementation)(HSquirrelVM* sqvm);

typedef struct SQFunctionRegistration {
  const char* sq_name;
  const char* native_name;
  const char* help_text;
  const char* raw_ret_ty;     // squirrel type signature of the return type
  const char* args_signature; // squirrel arguments signature
  uint32_t unknown1;
  uint32_t dev_level;
  const char* short_name;
  uint32_t unknown2;
  SQNativeClosureReturnType return_type;
  void* external_buffer;
  uint64_t external_buffer_size;
  uint64_t unknown3;
  uint64_t unknown4;
  SQNativeClosureImplementation implementation;
} SQFunctionRegistration;

typedef void* (*RegisterSquirrelFunctionTy)(CSquirrelVM* sqvm, void* fn, char bUnknown);

void cl_bind_native_closure(
    CSquirrelVM*, const char* return_type, const char* name, const char* args_signature, const char* help_text,
    SQNativeClosureImplementation implementation
);
void ui_bind_native_closure(
    CSquirrelVM*, const char* return_type, const char* name, const char* args_signature, const char* help_text,
    SQNativeClosureImplementation implementation
);
void sv_bind_native_closure(
    CSquirrelVM*, const char* return_type, const char* name, const char* args_signature, const char* help_text,
    SQNativeClosureImplementation implementation
);

void register_native_closures(CSquirrelVM*);
void cl_register_native_closures(CSquirrelVM*);
void ui_register_native_closures(CSquirrelVM*);
void sv_register_native_closures(CSquirrelVM*);

void deregister_native_closures(ScriptContext context);
void cl_deregister_native_closures();
void ui_deregister_native_closures();
void sv_deregister_native_closures();

#endif
