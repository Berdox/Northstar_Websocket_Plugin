#ifndef CVAR_H
#define CVAR_H

#include <stdbool.h>
#include <stdint.h>

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 128

typedef int CVarDllIdentifier_t;

// TODO moar headers
enum {
  COMMAND_MAX_ARGC = 64,
  COMMAND_MAX_LENGTH = 512,
};

typedef struct CCommand {
  struct {
  }* vftable;
  int64_t m_nArgc;
  int64_t m_nArgv0Size;
  char m_pArgSBuffer[COMMAND_MAX_LENGTH];
  char m_pArgvBuffer[COMMAND_MAX_LENGTH];
  const char* m_ppArgv[COMMAND_MAX_ARGC];
} CCommand;

typedef void (*FnCommandCallback_t)(const CCommand*);
typedef int (*FnCommandCompletionCallback
)(const char* partial,
  char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

struct ConCommandBase;

typedef struct IConCommandBaseAccessor {
  struct {
    bool (*RegisterConCommandBase)(struct ConCommandBase* pVar);
  }* vtable;
} IConCommandBaseAccessor;

typedef struct ConCommandBase {
  struct {
  }* vftable;
  void* m_pConCommandBaseVTable;
  struct ConCommandBase* m_pNext;
  bool m_bRegistered;
  char pad_0011[7];
  const char* m_pszName;
  const char* m_pszHelpString;
  struct ConCommandBase* s_pConCommandBases;
  IConCommandBaseAccessor* s_pAccessor; // IConCommandBaseAccessor
} ConCommandBase;

typedef void (*FnCommandCallback_t)(const CCommand* command);

typedef struct ConCommand {
  struct {
  }* vftable;

  ConCommandBase _base;
  FnCommandCallback_t m_pCommandCallback;
  FnCommandCompletionCallback m_pCompletionCallback;
  int m_nCallbackFlags;
  char pad_0054[4];
  int unk0;
  int unk2;
} ConCommand;

typedef struct ConVar {
  struct {
  }* vftable;

  ConCommandBase* m_ConCommandBase;
  const char* m_pszDefaultValue;
  struct {
    const char* m_pszString;
    int64_t iStringLength;
    float m_fValue;
    int m_nValue;
  } m_Value;
  bool m_bHasMin;
  float m_fMinVal;
  bool m_bHasMax;
  float m_fMaxVal;
  void* m_pMalloc;
  char m_pPad80[10];
} ConVar;

typedef struct SourceColor {
  unsigned char R, G, B, A;
} SourceColor;

typedef struct CCVarIteratorInternal {
  struct {
    void (*SetFirst)(struct CCVarIteratorInternal* self);
    void (*Next)(struct CCVarIteratorInternal* self);
    ConCommandBase* (*Get)(struct CCVarIteratorInternal* self);
  } vftable;
} CCVarIteratorInternal;

typedef struct ICvar {
  struct {
  }* IAppSystem_vftable;
  struct {
    CVarDllIdentifier_t (*AllocateDLLIdentifier)(struct ICvar* self);

    void (*RegisterConCommand
    )(struct ICvar* self, ConCommandBase* pCommandBase);
    void (*UnregisterConCommand
    )(struct ICvar* self, ConCommandBase* pCommandBase);
    void (*UnregisterConCommands)(struct ICvar* self, CVarDllIdentifier_t id);

    const char* (*GetCommandLineValue
    )(struct ICvar* self, const char* pVariableName);

    ConCommandBase* (*FindCommandBase)(struct ICvar* self, const char* name);
    const ConCommandBase* (*FindCommandBase_const
    )(const struct ICvar* self, const char* name);
    ConVar* (*FindVar)(struct ICvar* self, const char* name);
    const ConVar* (*FindVar_const)(const struct ICvar* self, const char* name);
    ConCommand* (*FindCommand)(struct ICvar* self, const char* name);
    const ConCommand* (*FindCommand_const
    )(const struct ICvar* self, const char* name);

    void (*sub_1)();
    void (*sub_2)();

    void (*sub_3)();
    void (*sub_4)();
    void (*sub_5)();

    void (*sub_6)();
    void (*sub_7)();

    void (*ConsoleColorPrintf
    )(struct ICvar* self, const SourceColor* clr, const char* pFormat, ...);
    void (*ConsolePrintf)(struct ICvar* self, const char* pFormat, ...);
    void (*ConsoleDPrintf)(struct ICvar* self, const char* pFormat, ...);

    void (*sub_8)();

    void (*sub_9)();

    void (*sub_10)();
    void (*sub_11)();

    void (*sub_12)();
    void (*sub_13)();

    void (*sub_14)();
    void (*sub_15)();
    void (*sub_16)();
    void (*sub_17)();
    void (*sub_18)();
    void (*sub_19)();

    CCVarIteratorInternal* (*FactoryInternalIterator)(struct ICvar* self);
  }* vftable;
} ICvar;

void engine_cvar_init();

#endif
