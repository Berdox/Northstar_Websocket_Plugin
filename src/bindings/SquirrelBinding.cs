using System;
using System.Runtime.InteropServices;
using static NorthstarWebsocketCsPlugin.NativeBinding;

namespace NorthstarWebsocketCsPlugin;


//*************************************************************************************************
// Sets what VM is going to run scriot
//*************************************************************************************************
internal enum ScriptContext {
    Server = 0,
    Client = 1,
    Ui = 2,
}

//*************************************************************************************************
// Interger return codes from Squirrel VM
//*************************************************************************************************
internal static class SQResult {
    // Returned when a native function encounters an error or raises an exception.
    public const int Error = -1;

    // Returned when a native function executes successfully and yields void/null.
    public const int Null = 0;

    // Returned when a native function executes successfully and pushes a return value onto the VM stack.
    public const int NotNull = 1;
}


//*************************************************************************************************
// BitMask for the engine for type checking for return types
//*************************************************************************************************
internal static class SqReturnType {
    public const uint Bool = 0x6;
    public const uint Array = 0x25;
    public const uint Default = 0x20; // void
}

//*************************************************************************************************
// Same as SQFunction Registration so the engine and plugin know the same C ABI structure
//*************************************************************************************************
[StructLayout(LayoutKind.Sequential)]
internal unsafe struct SQFunctionRegistration {
    // Pointer to null-terminated UTF-8 string for the script-facing function name.
    public byte* sq_name;

    // Pointer to UTF-8 native binding identifier name.
    public byte* native_name;

    // Pointer to UTF-8 help string describing function usage.
    public byte* help_text;

    // Pointer to UTF-8 script type signature string for return type.
    public byte* raw_ret_ty;

    // Pointer to UTF-8 argument type signature definition (e.g., "string socketName, string url").
    public byte* args_signature;

    // Unknown/padding member preserved for C ABI layout alignment.
    public uint unknown1;

    // Developer access level flags required to invoke the command (0 = public/unrestricted).
    public uint dev_level;

    // Pointer to UTF-8 shortened function name string.
    public byte* short_name;

    // Unknown/padding member preserved for C ABI layout alignment.
    public uint unknown2;

    // Engine return type flag matching values in <see cref="SqReturnType"/>.
    public uint return_type;

    // Reserved external buffer pointer for custom execution context data.
    public IntPtr external_buffer;

    // Size of the external buffer allocation.
    public ulong external_buffer_size;

    // Unknown/padding member preserved for C ABI layout alignment.
    public ulong unknown3;

    // Unknown/padding member preserved for C ABI layout alignment.
    public ulong unknown4;

    // Native unmanaged function pointer implementation: SQRESULT (*)(HSquirrelVM* sqvm)
    public IntPtr implementation;
}

//*************************************************************************************************
// DLL Squirrel API Surface
//*************************************************************************************************
internal unsafe struct SqApi {
    // Memory base address of the resolved target DLL module (server.dll or client.dll).
    public IntPtr ModuleBase;

    // Function pointer to sq_pushstring: pushes a UTF-8 string slice onto the VM stack.
    public delegate* unmanaged<IntPtr, byte*, int, void> sq_pushstring;

    // Function pointer to sq_pushbool: pushes a boolean flag onto the VM stack.
    public delegate* unmanaged<IntPtr, uint, void> sq_pushbool;

    // Function pointer to sq_getstring: fetches a UTF-8 string pointer from a stack index.
    public delegate* unmanaged<IntPtr, int, byte*> sq_getstring;

    // Function pointer to sq_getinteger: fetches a signed 32-bit integer from a stack index.
    public delegate* unmanaged<IntPtr, int, int> sq_getinteger;

    // Function pointer to sq_getbool: fetches a boolean flag from a stack index.
    public delegate* unmanaged<IntPtr, int, uint> sq_getbool;

    // Function pointer to sq_newarray: creates a new empty array on top of the VM stack.
    public delegate* unmanaged<IntPtr, uint, void> sq_newarray;

    // Function pointer to sq_arrayappend: appends the top stack item into an array at a target index.
    public delegate* unmanaged<IntPtr, int, int> sq_arrayappend;

    // Function pointer to the engine function registration routine.
    public delegate* unmanaged<IntPtr, IntPtr, byte, IntPtr> register_function;

    // Get a value indicating whether function pointers have been resovled for module
    public readonly bool IsReady => ModuleBase != IntPtr.Zero;
}

//*************************************************************************************************
// Helper methods to acess CSquirrelVM memory structures, resolving function pointer tables,
// registering native closures, and marshaling values acress the VM stack.
//*************************************************************************************************
internal static unsafe class SquirrelBinding {
    // Offset (0x08) within CSquirrelVM to get the underlying HSquirrelVM handle.
    private const int CSquirrelVM_SqvmOffset = 0x08;

    // Offset (0x3C) within CSquirrelVM storing the ScriptContext enum value.
    private const int CSquirrelVM_ContextOffset = 0x3C;

    //*************************************************************************************************
    // Reads HSquirrel VM pointer from offset in Squrrel VM instance
    // csqvm = Pointer to Squirrel VM
    // return = point to HSquirrel VM state
    //*************************************************************************************************
    public static IntPtr GetHSquirrelVm(IntPtr csqvm) => Marshal.ReadIntPtr(csqvm, CSquirrelVM_SqvmOffset);

    //*************************************************************************************************
    // Reads ScripContext enum from offset in Squirrel VM instance
    // csqvm = Pointer to Squirrel VM
    // return = VM context (Server, Client, UI)
    //*************************************************************************************************
    public static ScriptContext GetContext(IntPtr csqvm) => (ScriptContext)Marshal.ReadInt32(csqvm, CSquirrelVM_ContextOffset);

    //*************************************************************************************************
    // Gets the absolute memory address from byte offset to module base address pointer
    //*************************************************************************************************
    private static void* At(IntPtr moduleBase, int offset) => (void*)((byte*)moduleBase + offset);

    //*************************************************************************************************
    // Builds and populate the function table for Server Squirrel API surface for byte offsets
    //*************************************************************************************************
    public static SqApi BuildServerApi(IntPtr serverDllBase) {
        var api = new SqApi { ModuleBase = serverDllBase };

        // Resolve function pointer addresses relative to server.dll base address.
        api.sq_pushstring = (delegate* unmanaged<IntPtr, byte*, int, void>)At(serverDllBase, 0x3440);
        api.sq_pushbool = (delegate* unmanaged<IntPtr, uint, void>)At(serverDllBase, 0x3710);
        api.sq_getstring = (delegate* unmanaged<IntPtr, int, byte*>)At(serverDllBase, 0x60A0);
        api.sq_getinteger = (delegate* unmanaged<IntPtr, int, int>)At(serverDllBase, 0x60C0);
        api.sq_getbool = (delegate* unmanaged<IntPtr, int, uint>)At(serverDllBase, 0x6110);
        api.sq_newarray = (delegate* unmanaged<IntPtr, uint, void>)At(serverDllBase, 0x39F0);
        api.sq_arrayappend = (delegate* unmanaged<IntPtr, int, int>)At(serverDllBase, 0x3C70);
        api.register_function = (delegate* unmanaged<IntPtr, IntPtr, byte, IntPtr>)At(serverDllBase, 0x1DD10);

        return api;
    }

    //*************************************************************************************************
    //Builds and populate the function table for Client & UI Squirrel API surface for byte offsets
    //*************************************************************************************************
    public static SqApi BuildClientApi(IntPtr clientDllBase) {
        var api = new SqApi { ModuleBase = clientDllBase };

        // Resolve function pointer addresses relative to client.dll base address.
        api.sq_pushstring = (delegate* unmanaged<IntPtr, byte*, int, void>)At(clientDllBase, 0x3440);
        api.sq_pushbool = (delegate* unmanaged<IntPtr, uint, void>)At(clientDllBase, 0x3710);
        api.sq_getstring = (delegate* unmanaged<IntPtr, int, byte*>)At(clientDllBase, 0x60C0);
        api.sq_getinteger = (delegate* unmanaged<IntPtr, int, int>)At(clientDllBase, 0x60E0);
        api.sq_getbool = (delegate* unmanaged<IntPtr, int, uint>)At(clientDllBase, 0x6130);
        api.sq_newarray = (delegate* unmanaged<IntPtr, uint, void>)At(clientDllBase, 0x39F0);
        api.sq_arrayappend = (delegate* unmanaged<IntPtr, int, int>)At(clientDllBase, 0x3C70);
        api.register_function = (delegate* unmanaged<IntPtr, IntPtr, byte, IntPtr>)At(clientDllBase, 0x108E0);

        return api;
    }

    //*************************************************************************************************
    // Allocates memory. populates SQFunctionRegistration structure. Regusters C# method implementation
    // into the Squirrel VM instance
    //*************************************************************************************************
    public static void RegisterFunction(
        in SqApi api, IntPtr csqvm, string sqName, string rawRetTy, string argsSignature, string helpText,
        uint returnTypeCategory, IntPtr implementation) {
        var reg = (SQFunctionRegistration*)Marshal.AllocHGlobal(sizeof(SQFunctionRegistration));

        byte* namePtr = ToUtf8(sqName);
        reg->sq_name = namePtr;
        reg->native_name = namePtr;
        reg->short_name = namePtr;
        reg->help_text = ToUtf8(helpText);
        reg->raw_ret_ty = ToUtf8(rawRetTy);
        reg->args_signature = ToUtf8(argsSignature);

        reg->unknown1 = 0;
        reg->dev_level = 0;
        reg->unknown2 = 0;
        reg->return_type = returnTypeCategory;

        reg->external_buffer = IntPtr.Zero;
        reg->external_buffer_size = 0;
        reg->unknown3 = 0;
        reg->unknown4 = 0;

        reg->implementation = implementation;

        api.register_function(csqvm, (IntPtr)reg, 0);
    }

    //*************************************************************************************************
    // Retrieves and reads string argument from Squirrel VM evaluation stack at a index
    //*************************************************************************************************
    public static string GetStringArg(in SqApi api, IntPtr sqvm, int stackPos)
        => PtrToUtf8String(api.sq_getstring(sqvm, stackPos));

    //*************************************************************************************************
    // Makes C# string in UTF-8 bytes and pushes it into squirrel VM stack
    //*************************************************************************************************
    public static void PushString(in SqApi api, IntPtr sqvm, string s) {
        byte[] bytes = System.Text.Encoding.UTF8.GetBytes(s);

        fixed (byte* p = bytes) {
            api.sq_pushstring(sqvm, p, bytes.Length);
        }
    }
}