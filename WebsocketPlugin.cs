using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.WebSockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using static NorthstarWebsocketCsPlugin.NativeInterop;
using static NorthstarWebsocketCsPlugin.SquirrelInterop;

namespace NorthstarWebsocketCsPlugin;


//**************************************************************************************************
// Implements Native Squirrel script bindings
// Registers it with Squirrel VM (Server, Client, UI)
// Open sockets are left open and not closed on Squirrel VM destructuion
//**************************************************************************************************
internal static unsafe class WebsocketPlugin {

    // server context (server.dll).
    private static SqApi s_serverApi;

    // client and UI contexts (client.dll).
    private static SqApi s_clientApi;


    //**************************************************************************************************
    // Initalizes Squirrel API function pointers when DLL loaded
    // moduleName = DLL name
    // moduleBase = Memory base address pointer where DLL is loaded
    //**************************************************************************************************
    public static void OnLibraryLoaded(string moduleName, IntPtr moduleBase) {
        if (string.Equals(moduleName, "server.dll", StringComparison.OrdinalIgnoreCase) && !s_serverApi.IsReady) {
            s_serverApi = SquirrelInterop.BuildServerApi(moduleBase);
            DebugLog($"WebsocketPlugin: resolved server.dll squirrel API at 0x{moduleBase:X}");
        }

        else if (string.Equals(moduleName, "client.dll", StringComparison.OrdinalIgnoreCase) && !s_clientApi.IsReady) {
            s_clientApi = SquirrelInterop.BuildClientApi(moduleBase);
            DebugLog($"WebsocketPlugin: resolved client.dll squirrel API at 0x{moduleBase:X}");
        }
    }







    // --------------------------------- registration Function -----------------------------------------------------




    //**************************************************************************************************
    // Binds C# plugin function with new Squirrel VM instance
    // csqvn = pointer to Squirrel VM
    //**************************************************************************************************
    public static void RegisterForSqvm(IntPtr csqvm) {.
        ScriptContext context = SquirrelInterop.GetContext(csqvm);
        DebugLog($"WebsocketPlugin: OnSqvmCreated context={context}");

        switch (context) {
            case ScriptContext.Server:
                if (!s_serverApi.IsReady) {
                    IntPtr baseAddr = GetModuleHandleA("server.dll");
                    if (baseAddr != IntPtr.Zero)
                        OnLibraryLoaded("server.dll", baseAddr);
                }
                if (s_serverApi.IsReady)
                    RegisterServerFunctions(csqvm);
                break;

            case ScriptContext.Client:
            case ScriptContext.Ui:
                if (!s_clientApi.IsReady) {
                    IntPtr baseAddr = GetModuleHandleA("client.dll");
                    if (baseAddr != IntPtr.Zero)
                        OnLibraryLoaded("client.dll", baseAddr);
                }
                if (s_clientApi.IsReady)
                    RegisterClientFunctions(csqvm);
                break;
        }
    }

    //**************************************************************************************************
    // Register plugin script methods to Server VM table
    // csqvm = pointer to Squirrel VM
    //**************************************************************************************************
    private static void RegisterServerFunctions(IntPtr csqvm) {
        // Bind PL_ConnectToWebsocket
        RegisterFunction(in s_serverApi, csqvm, "PL_ConnectToWebsocket", "bool",
            "string socketName, string url, string header, int connectionTimeout, bool keepAlive",
            "Connects to a websocket by name. Returns true if connected.", SqReturnType.Bool,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_ConnectToWebsocket_Server);

        // Bind PL_DisconnectFromWebsocket.
        RegisterFunction(in s_serverApi, csqvm, "PL_DisconnectFromWebsocket", "void",
            "string socketName", "Disconnects a named websocket.", SqReturnType.Default,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_DisconnectFromWebsocket_Server);

        // Bind PL_WriteToWebsocket.
        RegisterFunction(in s_serverApi, csqvm, "PL_WriteToWebsocket", "bool",
            "string socketName, string message", "Sends a message on a named websocket.", SqReturnType.Bool,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_WriteToWebsocket_Server);

        // Bind PL_ReadFromWebsocket.
        RegisterFunction(in s_serverApi, csqvm, "PL_ReadFromWebsocket", "array<string>",
            "string socketName", "Returns all messages received since the last call.", SqReturnType.Array,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_ReadFromWebsocket_Server);

        // Bind PL_GetOpenWebsockets.
        RegisterFunction(in s_serverApi, csqvm, "PL_GetOpenWebsockets", "array<string>",
            "", "Returns the names of all currently open websockets.", SqReturnType.Array,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_GetOpenWebsockets_Server);
    }

    //**************************************************************************************************
    // Register plugin script methods to Client and UI VM table
    // csqvm = pointer to Squirrel VM
    //**************************************************************************************************
    private static void RegisterClientFunctions(IntPtr csqvm) {
        // Bind PL_ConnectToWebsocket
        RegisterFunction(in s_clientApi, csqvm, "PL_ConnectToWebsocket", "bool",
            "string socketName, string url, string header, int connectionTimeout, bool keepAlive",
            "Connects to a websocket by name. Returns true if connected.", SqReturnType.Bool,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_ConnectToWebsocket_Client);

        // Bind PL_DisconnectFromWebsocket.
        RegisterFunction(in s_clientApi, csqvm, "PL_DisconnectFromWebsocket", "void",
            "string socketName", "Disconnects a named websocket.", SqReturnType.Default,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_DisconnectFromWebsocket_Client);

        // Bind PL_WriteToWebsocket.
        RegisterFunction(in s_clientApi, csqvm, "PL_WriteToWebsocket", "bool",
            "string socketName, string message", "Sends a message on a named websocket.", SqReturnType.Bool,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_WriteToWebsocket_Client);

        // Bind PL_ReadFromWebsocket.
        RegisterFunction(in s_clientApi, csqvm, "PL_ReadFromWebsocket", "array<string>",
            "string socketName", "Returns all messages received since the last call.", SqReturnType.Array,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_ReadFromWebsocket_Client);

        // Bind PL_GetOpenWebsockets.
        RegisterFunction(in s_clientApi, csqvm, "PL_GetOpenWebsockets", "array<string>",
            "", "Returns the names of all currently open websockets.", SqReturnType.Array,
            (IntPtr)(delegate* unmanaged<IntPtr, int>)&PL_GetOpenWebsockets_Client);
    }

    // --------------------------------- native function  -----------------------


    //**************************************************************************************************
    // Entry point for server-side connection requests
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_ConnectToWebsocket_Server(IntPtr sqvm) {
        try { return ConnectCore(in s_serverApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_ConnectToWebsocket_Server EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for client and UI connection requests
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_ConnectToWebsocket_Client(IntPtr sqvm) {
        try { return ConnectCore(in s_clientApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_ConnectToWebsocket_Client EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for server-side disconnect requests
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_DisconnectFromWebsocket_Server(IntPtr sqvm) {
        try { return DisconnectCore(in s_serverApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_DisconnectFromWebsocket_Server EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for client/UI disconnect requests
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_DisconnectFromWebsocket_Client(IntPtr sqvm) {
        try { return DisconnectCore(in s_clientApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_DisconnectFromWebsocket_Client EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for server-side socket writes
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_WriteToWebsocket_Server(IntPtr sqvm) {
        try { return WriteCore(in s_serverApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_WriteToWebsocket_Server EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for client/UI socket writes
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_WriteToWebsocket_Client(IntPtr sqvm) {
        try { return WriteCore(in s_clientApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_WriteToWebsocket_Client EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for client/UI socket writes
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_ReadFromWebsocket_Server(IntPtr sqvm) {
        try { return ReadCore(in s_serverApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_ReadFromWebsocket_Server EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for client/UI inbox reads
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_ReadFromWebsocket_Client(IntPtr sqvm) {
        try { return ReadCore(in s_clientApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_ReadFromWebsocket_Client EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for querying active server socket handles
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_GetOpenWebsockets_Server(IntPtr sqvm) {
        try { return GetOpenCore(in s_serverApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_GetOpenWebsockets_Server EXCEPTION: {ex}"); return SQResult.Null; }
    }

    //**************************************************************************************************
    // Entry point for querying active client/UI socket handles
    //**************************************************************************************************
    [UnmanagedCallersOnly]
    private static int PL_GetOpenWebsockets_Client(IntPtr sqvm) {
        try { return GetOpenCore(in s_clientApi, sqvm); }
        catch (Exception ex) { DebugLog($"PL_GetOpenWebsockets_Client EXCEPTION: {ex}"); return SQResult.Null; }
    }











    // --------------------------------- shared logic -------------------------------------------------------




    //**************************************************************************************************
    // Extracts connection arguments from the Squirrel stack and attempts to open a WebSocket connection
    //**************************************************************************************************
    private static int ConnectCore(in SqApi api, IntPtr sqvm) {
        string socketName = GetStringArg(in api, sqvm, 1);
        string url = GetStringArg(in api, sqvm, 2);
        string headerStr = GetStringArg(in api, sqvm, 3);
        int timeoutSec = api.sq_getinteger(sqvm, 4);
        bool keepAlive = api.sq_getbool(sqvm, 5) != 0;

        bool connected = Connect(socketName, url, headerStr, timeoutSec, keepAlive);

        api.sq_pushbool(sqvm, connected ? 1u : 0u);
        return SQResult.NotNull;
    }

    //**************************************************************************************************
    // Reads socket name from Squirrel stack and tears down the corresponding connection
    //**************************************************************************************************
    private static int DisconnectCore(in SqApi api, IntPtr sqvm) {
        string socketName = GetStringArg(in api, sqvm, 1);
        Disconnect(socketName);
        return SQResult.Null; // Return void/null to script engine.
    }

    //**************************************************************************************************
    // Reads socket target and payload string, sending data over the wire
    //**************************************************************************************************
    private static int WriteCore(in SqApi api, IntPtr sqvm) {
        string socketName = GetStringArg(in api, sqvm, 1);
        string message = GetStringArg(in api, sqvm, 2);

        bool ok = Write(socketName, message);

        api.sq_pushbool(sqvm, ok ? 1u : 0u);
        return SQResult.NotNull;
    }

    //**************************************************************************************************
    // Drains queued messages for a given socket into a native Squirrel array and returns it to the VM stack
    //**************************************************************************************************
    private static int ReadCore(in SqApi api, IntPtr sqvm) {
        string socketName = GetStringArg(in api, sqvm, 1);

        api.sq_newarray(sqvm, 0);

        foreach (string message in DrainMessages(socketName)) {
            PushString(in api, sqvm, message);
            api.sq_arrayappend(sqvm, 2); // 2 refers to the array stack index relative to the top.
        }
        return SQResult.NotNull;
    }

    //**************************************************************************************************
    // Fetches all active socket registration keys and pushes them as a Squirrel array of strings
    //**************************************************************************************************
    private static int GetOpenCore(in SqApi api, IntPtr sqvm) {
        api.sq_newarray(sqvm, 0);
        foreach (string name in GetOpenSocketNames()) {
            PushString(in api, sqvm, name);
            api.sq_arrayappend(sqvm, 2);
        }
        return SQResult.NotNull;
    }








    // --------------------------------- connection management ------------------------------------------





    // Thread-safe repository storing all active sockets indexed.
    private static readonly ConcurrentDictionary<string, WsConnection> s_connections = new();

    //**************************************************************************************************
    // Creates, configures, and connects a new client websocket instance asynchronously
    // name = name of websocket
    // url = url to connecting websockt
    // headerStr = header string
    // timeoutSec = Seconds before timing out socket
    // keepAlive = keep alive even if VM is destroyed
    // return = True is connection is succesful or existing connection, otherwise false
    //**************************************************************************************************
    private static bool Connect(string name, string url, string headerStr, int timeoutSec, bool keepAlive) {
        if (keepAlive && s_connections.TryGetValue(name, out WsConnection? existing)
            && existing.Socket.State == WebSocketState.Open) {
            return true;
        }

        if (s_connections.TryRemove(name, out WsConnection? old))
            old.Close();

        var socket = new ClientWebSocket();

        foreach ((string key, string value) in ParseHeaders(headerStr)) {
            try { socket.Options.SetRequestHeader(key, value); }
            catch (Exception ex) { DebugLog($"Websocket '{name}' bad header '{key}': {ex.Message}"); }
        }

        Uri uri;
        try { uri = new Uri(url); }
        catch (Exception ex) {
            DebugLog($"Websocket '{name}' invalid url '{url}': {ex.Message}");
            socket.Dispose();
            return false;
        }

        using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(Math.Max(1, timeoutSec)));
        try {
            socket.ConnectAsync(uri, cts.Token).GetAwaiter().GetResult();
        }
        catch (Exception ex) {
            DebugLog($"Websocket '{name}' connect to '{url}' failed: {ex.Message}");
            socket.Dispose();
            return false;
        }

        var conn = new WsConnection(socket);
        s_connections[name] = conn;
        conn.StartReceiveLoop(name);

        DebugLog($"Websocket '{name}' connected to '{url}'");
        return true;
    }

    //**************************************************************************************************
    // Disconnects and disposes a named WebSocket if it exists in the active tracking table
    // name = websocket name
    //**************************************************************************************************
    private static void Disconnect(string name) {
        if (s_connections.TryRemove(name, out WsConnection? conn)) {
            conn.Close();
            DebugLog($"Websocket '{name}' disconnected");
        }
    }

    //**************************************************************************************************
    // Encodes a text payload to UTF-8 bytes and pushes it synchronously to the target socket
    // name = socket name
    // message = string payload
    //**************************************************************************************************
    private static bool Write(string name, string message) {
        if (!s_connections.TryGetValue(name, out WsConnection? conn) || conn.Socket.State != WebSocketState.Open)
            return false;

        try {
            byte[] bytes = Encoding.UTF8.GetBytes(message);
            conn.Socket.SendAsync(new ArraySegment<byte>(bytes), WebSocketMessageType.Text, true, CancellationToken.None)
                .GetAwaiter().GetResult();
            return true;
        }
        catch (Exception ex) {
            DebugLog($"Websocket '{name}' write failed: {ex.Message}");
            return false;
        }
    }

    //**************************************************************************************************
    //  Yields and removes all currently enqueued messages from the target connection's inbox queue
    // name = websocket name
    //**************************************************************************************************
    private static IEnumerable<string> DrainMessages(string name) {
        if (!s_connections.TryGetValue(name, out WsConnection? conn))
            yield break;

        while (conn.Inbox.TryDequeue(out string? msg))
            yield return msg;
    }

    //**************************************************************************************************
    // Returns an enumerable collection containing all registered connection name keys
    //**************************************************************************************************
    private static IEnumerable<string> GetOpenSocketNames() => s_connections.Keys;

    //**************************************************************************************************
    // Parses delimited key/value header strings into tuple key-value pairs.
    // Header string format: "key|#!#|value|#!#|key2|#!#|value2|#!#|"
    //**************************************************************************************************
    private static IEnumerable<(string Key, string Value)> ParseHeaders(string headerStr) {
        if (string.IsNullOrEmpty(headerStr))
            yield break;

        string[] parts = headerStr.Split("|#!#|", StringSplitOptions.RemoveEmptyEntries);
        for (int i = 0; i + 1 < parts.Length; i += 2)
            yield return (parts[i], parts[i + 1]);
    }

    //**************************************************************************************************
    // Disconnects and purges all active sockets across all contexts during plugin unload.
    //**************************************************************************************************
    public static void CloseAllConnections() {
        foreach (string key in s_connections.Keys) {
            if (s_connections.TryRemove(key, out WsConnection? conn))
                conn.Close();
        }
    }
}