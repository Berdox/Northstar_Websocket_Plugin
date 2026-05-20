#ifndef RELAY_H
#define RELAY_H

#include "bindings/native_closures.h"
#include "squirrel/squirrel.h"
#include <windows.h>

// for functionality that exists both in server.dll and client.dll
typedef struct {
  void (*sq_pushroottable)(SQVM sqvm);
  void (*sq_pushstring)(SQVM sqvm, const SQChar* str, SQInteger length);
  void (*sq_pushinteger)(SQVM sqvm, SQInteger i);
  void (*sq_pushfloat)(SQVM sqvm, SQFloat f);
  void (*sq_pushbool)(SQVM sqvm, SQBool b);
  void (*sq_pushasset)(SQVM sqvm, const SQChar* str, SQInteger length);
  void (*sq_pushvector)(SQVM sqvm, const SQFloat* vec);
  void (*sq_pushobject)(SQVM sqvm, SQObject* o);

  const SQChar* (*sq_getstring)(SQVM sqvm, SQInteger stackpos);
  SQInteger (*sq_getinteger)(SQVM sqvm, SQInteger stackpos);
  SQFloat (*sq_getfloat)(SQVM sqvm, SQInteger stackpos);
  SQBool (*sq_getbool)(SQVM sqvm, SQInteger stackpos);
  SQRESULT(*sq_get)
  (SQVM sqvm, SQInteger stackpos);
  SQRESULT(*sq_getasset)
  (SQVM sqvm, SQInteger stackpos, const char* out);
  SQFloat* (*sq_getvector)(SQVM sqvm, SQInteger stackpos);
  SQBool (*sq_getthisentity)(SQVM sqvm, void** ent);
  void (*sq_getobject)(SQVM sqvm, SQInteger stackpos, SQObject* out);
  SQRESULT(*sq_getfunction)
  (SQVM sqvm, SQInteger stackpos);
  SQRESULT(*sq_getuserdata)
  (SQVM sqvm, SQInteger stackpos, void** data, int64_t* tag);

  void (*sq_newarray)(SQVM sqvm, SQUnsignedInteger size);
  SQRESULT(*sq_arrayappend)
  (SQVM sqvm, SQInteger stackpos);

  SQRESULT (*sq_newtable)(SQVM sqvm);
  SQRESULT (*sq_newslot)(SQVM sqvm, SQInteger stackpos, SQBool bStatic);

  SQRESULT (*sq_newstruct)(SQVM sqvm, SQUnsignedInteger size);
  SQRESULT (*sq_setmember)(SQVM sqvm, SQUnsignedInteger memberIndex);

  void* (*sq_createuserdata)(SQVM sqvm, SQUnsignedInteger size);
  SQRESULT(*sq_setuserdatatypetag)
  (SQVM sqvm, SQInteger stackpos, int64_t typeId);

  void* (*sq_stackinfos)(SQVM sqvm, SQInteger stackpos, void* out);
  SQRESULT (*sq_compilebuffer)(SQVM sqvm, void* buffer, const char* file, int unk, SQBool bShouldThrowError);
  SQRESULT (*sq_call)(SQVM sqvm, SQUnsignedInteger nArgs, SQBool bShouldReturn, SQBool bShouldThrowError);
  SQInteger (*sq_raiseerror)(SQVM sqvm, const SQChar* err);

  void (*c_sq_define_constant)(CSquirrelVM* c_sqvm, const SQChar* name, int i);
  RegisterSquirrelFunctionTy c_sq_register_function;
} SQRelay;

extern SQRelay g_sqsv;
extern SQRelay g_sqcl;

SQRelay* sqapi(ScriptContext context);

void init_relay_sv(HMODULE module);
void init_relay_cl(HMODULE module);

#endif
