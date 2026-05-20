#include "relay.h"
#include "interfaces/sys.h"

#define PTR_AT(offset) ((void*)(base + offset))

SQRelay g_sv = {0};
SQRelay g_sqcl = {0};

// get relay for squirrel context
SQRelay* sqapi(ScriptContext context) {
  switch (context) {
  case SC_SERVER:
    return &g_sv;
  case SC_CLIENT:
  case SC_UI:
    return &g_sqcl;
  default:
    return 0;
  }
}

void init_squirrel_relay_sv(uintptr_t base) {
  g_sv.sq_pushroottable = PTR_AT(0x5840);
  g_sv.sq_pushstring = PTR_AT(0x3440);
  g_sv.sq_pushinteger = PTR_AT(0x36a0);
  g_sv.sq_pushfloat = PTR_AT(0x3800);
  g_sv.sq_pushbool = PTR_AT(0x3710);
  g_sv.sq_pushasset = PTR_AT(0x3560);
  g_sv.sq_pushvector = PTR_AT(0x3780);
  g_sv.sq_pushobject = PTR_AT(0x83a0);

  g_sv.sq_getstring = PTR_AT(0x60a0);
  g_sv.sq_getinteger = PTR_AT(0x60c0);
  g_sv.sq_getfloat = PTR_AT(0x60e0);
  g_sv.sq_getbool = PTR_AT(0x6110);
  g_sv.sq_getasset = PTR_AT(0x5ff0);
  g_sv.sq_get = PTR_AT(0x7c00);
  g_sv.sq_getvector = PTR_AT(0x6120);
  g_sv.sq_getthisentity = PTR_AT(0x203B0);
  g_sv.sq_getobject = PTR_AT(0x6140);
  g_sv.sq_getfunction = PTR_AT(0x6c80);
  g_sv.sq_getuserdata = PTR_AT(0x63b0);

  g_sv.sq_newarray = PTR_AT(0x39F0);
  g_sv.sq_arrayappend = PTR_AT(0x3c70);

  g_sv.sq_newtable = PTR_AT(0x3960);
  g_sv.sq_newslot = PTR_AT(0x7080);

  g_sv.sq_newstruct = PTR_AT(0x53e0);
  g_sv.sq_setmember = PTR_AT(0x5510);

  g_sv.sq_createuserdata = PTR_AT(0x38d0);
  g_sv.sq_setuserdatatypetag = PTR_AT(0x6470);

  g_sv.sq_stackinfos = PTR_AT(0x35920);
  g_sv.sq_compilebuffer = PTR_AT(0x3110);
  g_sv.sq_call = PTR_AT(0x8620);
  g_sv.sq_raiseerror = PTR_AT(0x8440);

  g_sv.c_sq_define_constant = PTR_AT(0x1f550);
  g_sv.c_sq_register_function = PTR_AT(0x1DD10);
}

void init_relay_sv(HMODULE module) {
  uintptr_t base = (uintptr_t)module;

  init_squirrel_relay_sv(base);
}

void init_squirrel_relay_cl(uintptr_t base) {
  g_sqcl.sq_pushroottable = PTR_AT(0x5860);
  g_sqcl.sq_pushstring = PTR_AT(0x3440);
  g_sqcl.sq_pushinteger = PTR_AT(0x36a0);
  g_sqcl.sq_pushfloat = PTR_AT(0x3800);
  g_sqcl.sq_pushbool = PTR_AT(0x3710);
  g_sqcl.sq_pushasset = PTR_AT(0x3560);
  g_sqcl.sq_pushvector = PTR_AT(0x3780);
  g_sqcl.sq_pushobject = PTR_AT(0x83d0);

  g_sqcl.sq_getstring = PTR_AT(0x60c0);
  g_sqcl.sq_getinteger = PTR_AT(0x60e0);
  g_sqcl.sq_getfloat = PTR_AT(0x6100);
  g_sqcl.sq_getbool = PTR_AT(0x6130);
  g_sqcl.sq_getasset = PTR_AT(0x6010);
  g_sqcl.sq_get = PTR_AT(0x7c30);
  g_sqcl.sq_getvector = PTR_AT(0x6140);
  g_sqcl.sq_getthisentity = PTR_AT(0x12f80);
  g_sqcl.sq_getobject = PTR_AT(0x6160);
  g_sqcl.sq_getfunction = PTR_AT(0x6cb0);
  g_sqcl.sq_getuserdata = PTR_AT(0x63d0);

  g_sqcl.sq_newarray = PTR_AT(0x39F0);
  g_sqcl.sq_arrayappend = PTR_AT(0x3c70);

  g_sqcl.sq_newtable = PTR_AT(0x3960);
  g_sqcl.sq_newslot = PTR_AT(0x70b0);

  g_sqcl.sq_newstruct = PTR_AT(0x5400);
  g_sqcl.sq_setmember = PTR_AT(0x5530);

  g_sqcl.sq_createuserdata = PTR_AT(0x38d0);
  g_sqcl.sq_setuserdatatypetag = PTR_AT(0x6490);

  g_sqcl.sq_stackinfos = PTR_AT(0x35970);
  g_sqcl.sq_compilebuffer = PTR_AT(0x3110);
  g_sqcl.sq_call = PTR_AT(0x8650);
  g_sqcl.sq_raiseerror = PTR_AT(0x8740);

  g_sqcl.c_sq_define_constant = PTR_AT(0x12120);
  g_sqcl.c_sq_register_function = PTR_AT(0x108E0);
}

void init_relay_cl(HMODULE module) {
  uintptr_t base = (uintptr_t)module;
  init_squirrel_relay_cl(base);
}
