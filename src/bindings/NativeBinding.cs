using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace northstar_websocket_plugin.src.bindings {

    //*************************************************************************************************
    // Interchange of helpers for plugin, SuirrelInterop, and WebsockPlugin
    //*************************************************************************************************
    internal static unsafe class NativeBinding {
        //*************************************************************************************************
        // Gets the addres of exported function of variable from a DLL
        // gModule = handle to dll
        // procName = function or variable name to find
        // return = pointer to exported function or variable
        //*************************************************************************************************
        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        //*************************************************************************************************
        // Get module hande for spefific module if has be loaded
        // lpModuleName = name of loaded moudle (DLL)
        // return = handle to loaded module
        //*************************************************************************************************
        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
        public static extern IntPtr GetModuleHandleA(string? lpModuleName);

        //*************************************************************************************************
        // Allocates a permanent null-terminated UTF-8 buffer for managed string.
        // Used for native engine to keep a pointer to interfaces names, squirrel function metadata, etc
        // s = string to encode and allocate memory
        // return = point to permently allocated UTF-8 byte array in memory
        //*************************************************************************************************
        public static byte* ToUtf8(string s) {
            byte[] bytes = Encoding.UTF8.GetBytes(s + "\0");

            IntPtr ptr = Marshal.AllocHGlobal(bytes.Length);

            Marshal.Copy(bytes, 0, ptr, bytes.Length);

            return (byte*)ptr;
        }

        //*************************************************************************************************
        // Reads UTF-8 string from memory
        // p = pointer to byte array
        // return = decoded C# string or Empty, if pointer is null or unreadable
        //*************************************************************************************************
        public static string PtrToUtf8String(byte* p) {
            if (p == null)
                return string.Empty;

            try {
                return Marshal.PtrToStringUTF8((IntPtr)p) ?? string.Empty;
            }
            catch {
                return string.Empty;
            }
        }

        //*************************************************************************************************
        // File path to write diagnositc trace logs
        //*************************************************************************************************
        private static readonly string s_debugLogPath =
            Path.Combine(Path.GetTempPath(), "NorthstarWebsocketCsPlugin_debug.log");

        //*************************************************************************************************
        // Synchoronization object lock to multi-threade write access to debug log
        //*************************************************************************************************
        private static readonly object s_logLock = new();

        //*************************************************************************************************
        // Writes a timestamped message to trace logs
        //*************************************************************************************************
        public static void DebugLog(string msg) {
            try {
                lock (s_logLock) {
                    File.AppendAllText(s_debugLogPath, $"{DateTime.Now:HH:mm:ss.fff} {msg}{Environment.NewLine}");
                }
            }
            catch {
                // Suppress all exceptions; logging calls must remain robust and never crash host process.
            }
        }
    }
}