#pragma once
#include <Windows.h>

#define PLUGIN_CALLBACKS_VERSION "PluginCallbacks001"

// NS passes the plugin an instance of this struct in the Init callback
struct PluginNorthstarData
{
	HMODULE pluginHandle; // the HMODULE of this plugin. Used only in the NSSys interface rn.
};

class IPluginCallbacks
{
public:
	// Runs after the plugin is loaded and validated by NS
	// This is a great place to set up any interface you need from Northstar, for example NSSys001
	//
	// IPluginCallbacks::Init is guaranteed to be called before any other callback.
	virtual void Init(HMODULE northstarModule, const PluginNorthstarData* initData, bool reloaded) = 0;

	// Called after all plugins have been loaded.
	// If you depend on the interface of a different plugin, here is the place to get the handle of the plugin and initialize the interfaces.
	virtual void Finalize() = 0;

	// Right before this plugin is unloaded, this is called.
	// If you want your plugin to be reloadable at runtime, destroy / reset all your state like hooks etc here.
	virtual void Unload() = 0;

	// Every time a SERVER, CLIENT or UI sqvm is created this callback is triggered.
	// If you want to define a squirrel function, constant or similar, do it here.
	//
	// CSquirrelVM* c_sqvm
	virtual void OnSqvmCreated(void* c_sqvm) = 0;

	// Right before any sqvm is destroyed this is called
	//
	// CSquirrelVM* c_sqvm
	virtual void OnSqvmDestroying(void* c_sqvm) = 0;

	// Every time a module is loaded, plugins are notified with this callback.
	// This includes other plugins and binaries from the game like server.dll
	//
	// DO NOT CHECK IF A PLUGIN IS LOADED WITH THIS CALLBACK
	// This Plugin might be loaded *after* the plugin you're looking for. In this case your plugin will *not* get notified.
	// Use IPluginCallbacks::Finalize for this purpose.
	//
	// HMODULE module is the module handle aka base image pointer.
	// const char* name is the name of the module. This includes the file ending. For example "server.dll" or "client.dll".
	virtual void OnLibraryLoaded(HMODULE module, const char* name) = 0;

	// Called every game frame
	virtual void RunFrame() = 0;
};