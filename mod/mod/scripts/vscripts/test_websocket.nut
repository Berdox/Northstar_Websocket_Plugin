untyped

global function TestSockets_Init

void function TestSockets_Init()
{
    print("=== websocket test thread starting ===")
    thread TestSocket_Run()
}

void function TestSocket_Run()
{
    bool ok = NS_ConnectToWebsocket(
        "test_socket",
        "ws://localhost:9001/tf2",
        void function(string message) { print("[TestSocket] Received: " + message) },
        1
    )

    if (!ok)
    {
        print("[TestSocket] Failed to connect")
        return
    }

    print("[TestSocket] Connected, sending 'test' every 2 seconds")

    while (true)
    {
        wait 2
        NS_WriteToWebsocket("test_socket", "test")
    }
}
