untyped

global function TestSocket_Init

void function TestSocket_Init()
{
    printl( "******************************Test thread******************************" )
    thread TestSocket_Run()
}

void function TestSocket_Run()
{
    bool ok = NS_ConnectToWebsocket(
        "test_socket",
        "ws://localhost:8080",
        void function(string message) { printl("[TestSocket] Received: " + message) },
        1
    )

    if (!ok)
    {
        printl("******************************[TestSocket] Failed to connect******************************")
        return
    }

    printl("[TestSocket] Connected, sending 'test' every 2 seconds")

    while (true)
    {
        wait 2
        printl( "******************************Sending Test******************************" )
        NS_WriteToWebsocket("test_socket", "test")
    }
}