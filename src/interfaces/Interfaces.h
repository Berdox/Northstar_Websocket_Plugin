#pragma once

#define EXPORT_DLL extern "C" __declspec(dllexport)

enum class InterfaceStatus : int
{
	OK = 0,
	FAILED = 1,
};

EXPORT_DLL void* CreateInterface(const char* pName, InterfaceStatus* pReturnCode);

typedef void* (*InstantiateInterfaceFn)();


// Used internally to register classes.
class InterfaceReg
{
public:
	InterfaceReg(InstantiateInterfaceFn fn, const char* pName);

	InstantiateInterfaceFn m_CreateFn;
	const char* m_pName;
	InterfaceReg* m_pNext;
};

// Use this to expose an interface that can have multiple instances.
#define EXPOSE_INTERFACE(className, interfaceName, versionName)                                                                            \
	static void* __Create##className##_interface()                                                                                         \
	{                                                                                                                                      \
		return static_cast<interfaceName*>(new className);                                                                                 \
	}                                                                                                                                      \
	static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName);

#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName)                                            \
	static void* __Create##className##interfaceName##_interface()                                                                          \
	{                                                                                                                                      \
		return static_cast<interfaceName*>(&globalVarName);                                                                                \
	}                                                                                                                                      \
	static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName);

// Use this to expose a singleton interface. This creates the global variable for you automatically.
#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName)                                                                     \
	static className __g_##className##_singleton;                                                                                          \
	EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)