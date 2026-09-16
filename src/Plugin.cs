using System;
using System.Runtime.InteropServices;
using System.Text;
using northstar_websocket_plugin.src.bindings;
using static northstar_websocket_plugin.src.bindings.NativeBinding;
using static northstar_websocket_plugin.src.bindings.SquirrelBinding;

namespace northstar_websocket_plugin.src {

    //**************************************************************************************************
    // Exposes C++ ABI vtable, handles life-cycle callback and logging
    //**************************************************************************************************
    public static unsafe class Plugin {
        // Native instance pointer for the PluginId001 C++ interface (points to its vtable pointer).
        private static IntPtr s_pluginIdInstance;

        // Native instance pointer for the PluginCallbacks001 C++ interface (points to its vtable pointer).
        private static IntPtr s_pluginCallbacksInstance;

        // Handle to Northstar launcher binary (Northstar.dll).
        private static IntPtr s_northstarModule;

        // Handle to this compiled C# NativeAOT DLL instance.
        private static IntPtr s_ownModule;

        // Pointer to Northstar's core subsystem interface (NSSys001).
        private static IntPtr s_sysInterface;


        //**************************************************************************************************
        // Exported entry point
        // name = interface name
        // status = status code, 0 = success, 1 = error
        // return = Point to instance to the requested interface
        //*************************************************************************************************

        [UnmanagedCallersOnly(EntryPoint = "CreateInterface")]
        public static IntPtr CreateInterface(byte* name, int* status) {
            try {
                string? interfaceName = Marshal.PtrToStringAnsi((IntPtr)name);
                DebugLog($"CreateInterface called with name='{interfaceName}'");

                switch (interfaceName) {
                    case "PluginId001":
                        if (status != null) *status = 0; // Return success status.
                        return GetPluginIdInstance();

                    case "PluginCallbacks001":
                        if (status != null) *status = 0; // Return success status.
                        return GetPluginCallbacksInstance();

                    default:
                        if (status != null) *status = 1; // Return failure status.
                        return IntPtr.Zero;
                }
            }
            catch (Exception ex) {
                DebugLog($"CreateInterface EXCEPTION: {ex}");
                if (status != null) *status = 1;
                return IntPtr.Zero;
            }
        }


        //**************************************************************************************************
        // Plugin ID GetString
        // self = Pointer to interface instance
        // id = Enumerator ID for what string property
        // return = pointer to a string
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static byte* PluginId_GetString(IntPtr self, int id) {
            try {
                return id switch {
                    0 => s_nameStr,             // NAME identifier string.
                    1 => s_logNameStr,          // LOG_NAME (8-char prefix string for engine console).
                    2 => s_dependencyNameStr,   // DEPENDENCY_NAME (registered Squirrel dependency).
                    _ => null,
                };
            }
            catch (Exception ex) {
                DebugLog($"PluginId_GetString EXCEPTION: {ex}");
                return null;
            }
        }

        //**************************************************************************************************
        // Plugin ID GetField
        // self = Pointer to interface instance
        // id = Enumerator ID for what numeric field
        // return = 64 bit interger
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static long PluginId_GetField(IntPtr self, int id) {
            try {
                return id switch {
                    // CONTEXT: Combined flags (0x1 = DEDICATED, 0x2 = CLIENT) -> load in all VM contexts.
                    0 => 0x1 | 0x2,
                    1 => 0, // COLOR: 0 = default console log color.
                    _ => 0,
                };
            }
            catch (Exception ex) {
                DebugLog($"PluginId_GetField EXCEPTION: {ex}");
                return 0;
            }
        }


        //**************************************************************************************************
        // C++ ABI Instance and vtable of plugin instance
        // return = point to native interface structure
        //**************************************************************************************************

        private static IntPtr GetPluginIdInstance() {
            if (s_pluginIdInstance != IntPtr.Zero)
                return s_pluginIdInstance;

            IntPtr vtable = Marshal.AllocHGlobal(IntPtr.Size * 2);

            Marshal.WriteIntPtr(vtable, 0 * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, int, byte*>)&PluginId_GetString);

            Marshal.WriteIntPtr(vtable, 1 * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, int, long>)&PluginId_GetField);

            IntPtr instance = Marshal.AllocHGlobal(IntPtr.Size);

            Marshal.WriteIntPtr(instance, vtable);

            s_pluginIdInstance = instance;
            return instance;
        }

        //**************************************************************************************************
        // Northstar initialization parameters
        //**************************************************************************************************

        [StructLayout(LayoutKind.Sequential)]
        private struct PluginNorthstarData {
            public IntPtr pluginHandle;
        }

        //**************************************************************************************************
        // Initizlizes Plugin Callback init. 
        // self = Pointer to interface instance
        // northstarModule = Module handle for northstar
        // initData =  Pointer to initilized data block
        // reloaded = Flag to say if plugin reloaded on runtime
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_Init(IntPtr self, IntPtr northstarModule, IntPtr initData, byte reloaded) {
            try {
                DebugLog($"Callbacks_Init(northstarModule=0x{northstarModule:X}, reloaded={reloaded})");

                s_northstarModule = northstarModule;

                if (initData != IntPtr.Zero) {
                    var data = (PluginNorthstarData*)initData;
                    s_ownModule = data->pluginHandle;
                }

                IntPtr createInterfaceAddr = GetProcAddress(northstarModule, "CreateInterface");
                if (createInterfaceAddr != IntPtr.Zero) {
                    var northstarCreateInterface = (delegate* unmanaged<byte*, int*, IntPtr>)createInterfaceAddr;

                    int status;
                    IntPtr sys = northstarCreateInterface(s_nsSysNameStr, &status);
                    DebugLog($"  NSSys001 lookup -> ptr=0x{sys:X} status={status}");

                    if (status == 0 && sys != IntPtr.Zero) {
                        s_sysInterface = sys;
                        LogToConsole(reloaded != 0
                            ? "Northstar Websocket Plugin reloaded!"
                            : "Northstar Websocket Plugin loaded!");
                    }
                }

                IntPtr serverModule = GetModuleHandleA("server.dll");
                if (serverModule != IntPtr.Zero)
                    WebsocketPlugin.OnLibraryLoaded("server.dll", serverModule);

                IntPtr clientModule = GetModuleHandleA("client.dll");
                if (clientModule != IntPtr.Zero)
                    WebsocketPlugin.OnLibraryLoaded("client.dll", clientModule);
            }
            catch (Exception ex) {
                DebugLog($"Callbacks_Init EXCEPTION: {ex}");
            }
        }


        //**************************************************************************************************
        // Implements Plugin Callback Final function
        // self = Pointer to interface instance
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_Finalize(IntPtr self) {
            try { DebugLog("Callbacks_Finalize called"); }
            catch { /* Catch all exceptions at unmanaged entry boundary */ }
        }

        //**************************************************************************************************
        // Implements Plugin Callback unload function
        // self = Pointer to interface instance
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static byte Callbacks_Unload(IntPtr self) {
            try {
                DebugLog("Callbacks_Unload called");

                // Safely tear down and close active WebSocket sockets.
                WebsocketPlugin.CloseAllConnections();
            }
            catch { /* Catch all exceptions at unmanaged entry boundary */ }
            return 1;
        }


        //**************************************************************************************************
        // Implemets plugin Callback SQVM created
        // self = Pointer to interface instance
        // csqvm = pointer to CSquirrelVM
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_OnSqvmCreated(IntPtr self, IntPtr csqvm) {
            try {
                // Bind native plugin closures to the new VM instance.
                WebsocketPlugin.RegisterForSqvm(csqvm);
            }
            catch (Exception ex) {
                DebugLog($"Callbacks_OnSqvmCreated EXCEPTION: {ex}");
            }
        }


        //**************************************************************************************************
        // Implemets plugin Callback SQVM destroy
        // self = Pointer to interface instance
        // csqvm = pointer to CSquirrelVM
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_OnSqvmDestroying(IntPtr self, IntPtr csqvm) {
            // open websockets persist independently across VM context destructions.
            try { /* Catch all exceptions at unmanaged entry boundary */ } catch { }
        }


        //**************************************************************************************************
        // Implemets plugin callback libray loaded
        // self = Pointer to interface instance
        // module = OS handle to loaded library
        // name = pointer to module name
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_OnLibraryLoaded(IntPtr self, IntPtr module, byte* name) {
            try {
                string moduleName = PtrToUtf8String(name);

                // Pass resolved module load event to main plugin logic.
                WebsocketPlugin.OnLibraryLoaded(moduleName, module);
            }
            catch (Exception ex) {
                DebugLog($"Callbacks_OnLibraryLoaded EXCEPTION: {ex}");
            }
        }

        //**************************************************************************************************
        // Implements plugin callback runFrame
        // self = Pointer to interface instance
        //**************************************************************************************************

        [UnmanagedCallersOnly]
        private static void Callbacks_RunFrame(IntPtr self) {
            try { /* Keep blank to minimize performance overhead per frame */ } catch { }
        }

        //**************************************************************************************************
        // Implemets plugin Callback Instance
        //**************************************************************************************************

        private static IntPtr GetPluginCallbacksInstance() {
            if (s_pluginCallbacksInstance != IntPtr.Zero)
                return s_pluginCallbacksInstance;

            const int methodCount = 7;

            IntPtr vtable = Marshal.AllocHGlobal(IntPtr.Size * methodCount);
            int i = 0;

            // Write sequential function pointers into vtable memory layout matching native struct declaration.
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, IntPtr, IntPtr, byte, void>)&Callbacks_Init);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, void>)&Callbacks_Finalize);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, byte>)&Callbacks_Unload);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, IntPtr, void>)&Callbacks_OnSqvmCreated);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, IntPtr, void>)&Callbacks_OnSqvmDestroying);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, IntPtr, byte*, void>)&Callbacks_OnLibraryLoaded);
            Marshal.WriteIntPtr(vtable, (i++) * IntPtr.Size,
                (IntPtr)(delegate* unmanaged<IntPtr, void>)&Callbacks_RunFrame);

            IntPtr instance = Marshal.AllocHGlobal(IntPtr.Size);

            Marshal.WriteIntPtr(instance, vtable);

            s_pluginCallbacksInstance = instance;
            return instance;
        }


        //**************************************************************************************************
        // Logs to Northstar console
        // msg = Message string
        // level = severity level (0 = Info, 1 = Warning, 2 = Error)
        //**************************************************************************************************

        internal static void LogToConsole(string msg, int level = 0 /* LOG_INFO */) {
            if (s_sysInterface == IntPtr.Zero)
                return;

            IntPtr vtable = Marshal.ReadIntPtr(s_sysInterface);

            IntPtr logFnPtr = Marshal.ReadIntPtr(vtable, 0 * IntPtr.Size);

            var log = (delegate* unmanaged<IntPtr, IntPtr, int, byte*, void>)logFnPtr;

            byte[] utf8 = Encoding.UTF8.GetBytes(msg + "\0");

            fixed (byte* msgPtr = utf8) {
                log(s_sysInterface, s_ownModule, level, msgPtr);
            }
        }

        // Plugin different names.
        private static readonly byte* s_nameStr = ToUtf8("NS_Websocket");
        private static readonly byte* s_logNameStr = ToUtf8("NWSocket");
        private static readonly byte* s_dependencyNameStr = ToUtf8("NS_WEBSOCKET_PLUGIN");
        private static readonly byte* s_nsSysNameStr = ToUtf8("NSSys001");
    }
}