import asyncio
import ctypes

# Import websockets server library
import websockets

# Path to compiled C-ABI DLL export
DLL_PATH = "./bin/Release/net8.0/win-x64/northstar_websocket_plugin.dll"

# --- 1. Load DLL and Configure Signature Signatures ---
plugin = ctypes.CDLL(DLL_PATH)

# PL_ConnectToWebsocket
plugin.PL_ConnectToWebsocket.argtypes = [
    ctypes.c_char_p,  # pSocketName
    ctypes.c_char_p,  # pUrl
    ctypes.c_char_p,  # pHeader
    ctypes.c_int,  # connectionTimeout
    ctypes.c_uint8,  # keepAlive (0 or 1)
]
plugin.PL_ConnectToWebsocket.restype = ctypes.c_uint8

# PL_DisconnectFromWebsocket
plugin.PL_DisconnectFromWebsocket.argtypes = [ctypes.c_char_p]
plugin.PL_DisconnectFromWebsocket.restype = None

# PL_WriteToWebsocket
plugin.PL_WriteToWebsocket.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
plugin.PL_WriteToWebsocket.restype = None

# PL_ReadFromWebsocket
plugin.PL_ReadFromWebsocket.argtypes = [
    ctypes.c_char_p,  # pSocketName
    ctypes.c_char_p,  # outBuffer
    ctypes.c_int,  # bufferSize
]
plugin.PL_ReadFromWebsocket.restype = ctypes.c_int

# PL_GetOpenWebsockets
plugin.PL_GetOpenWebsockets.argtypes = [
    ctypes.c_char_p,  # outBuffer
    ctypes.c_int,  # bufferSize
]
plugin.PL_GetOpenWebsockets.restype = ctypes.c_int


# --- 2. Asynchronous WebSocket Server ---
async def websocket_handler(websocket):
    print("\n[Server] Client connected successfully!")
    try:
        async for message in websocket:
            print(f"[Server] Received message from DLL: '{message}'")
            # Echo back a response to test PL_ReadFromWebsocket
            response = f"Ack: {message}"
            await websocket.send(response)
            print(f"[Server] Sent response to DLL: '{response}'")
    except websockets.ConnectionClosed:
        print("[Server] Client disconnected.")


async def start_server(host="127.0.0.1", port=8080):
    async with websockets.serve(websocket_handler, host, port):
        print(f"[Server] Running at ws://{host}:{port}")
        await asyncio.Future()  # Keep server running


# --- 3. DLL Integration Test Suite ---
async def run_tests():
    # Allow server time to bind
    await asyncio.sleep(0.5)

    socket_name = b"TestSocket"
    url = b"ws://127.0.0.1:8080"
    header = b"Authorization: Bearer test_token"
    buffer_size = 1024
    buf = ctypes.create_string_buffer(buffer_size)

    print("\n--- Starting DLL Function Tests ---")

    # Test 1: PL_ConnectToWebsocket
    print("[DLL Test] Calling PL_ConnectToWebsocket...")
    conn_result = plugin.PL_ConnectToWebsocket(
        socket_name, url, header, 5000, 1
    )
    print(
        f"[DLL Test] PL_ConnectToWebsocket returned: {conn_result} (Success: {conn_result == 1})"
    )

    await asyncio.sleep(0.5)

    # Test 2: PL_GetOpenWebsockets
    print("\n[DLL Test] Calling PL_GetOpenWebsockets...")
    socket_count = plugin.PL_GetOpenWebsockets(buf, buffer_size)
    print(
        f"[DLL Test] Open Sockets Count: {socket_count}, Buffer: {buf.value.decode('utf-8')}"
    )

    # Test 3: PL_WriteToWebsocket
    print("\n[DLL Test] Calling PL_WriteToWebsocket...")
    message = b'{"event": "ping", "payload": "hello_from_dll"}'
    plugin.PL_WriteToWebsocket(socket_name, message)

    # Wait briefly for server echo
    await asyncio.sleep(0.5)

    # Test 4: PL_ReadFromWebsocket
    print("\n[DLL Test] Calling PL_ReadFromWebsocket...")
    msg_count = plugin.PL_ReadFromWebsocket(socket_name, buf, buffer_size)
    print(
        f"[DLL Test] Read Messages Count: {msg_count}, JSON Payload: {buf.value.decode('utf-8')}"
    )

    # Test 5: PL_DisconnectFromWebsocket
    print("\n[DLL Test] Calling PL_DisconnectFromWebsocket...")
    plugin.PL_DisconnectFromWebsocket(socket_name)
    await asyncio.sleep(0.5)

    # Verify disconnection
    socket_count = plugin.PL_GetOpenWebsockets(buf, buffer_size)
    print(
        f"[DLL Test] Open Sockets Count post-disconnect: {socket_count}, Buffer: {buf.value.decode('utf-8')}"
    )


async def main():
    # Run the server and test suite concurrently
    server_task = asyncio.create_task(start_server())
    test_task = asyncio.create_task(run_tests())

    await test_task
    server_task.cancel()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except asyncio.CancelledError:
        pass