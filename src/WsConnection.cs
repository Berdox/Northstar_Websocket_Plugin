using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Net.WebSockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;


//*************************************************************************************************
// Wraps named websockert connection: socket itself, queue of messages received, and background
// receive loop
//*************************************************************************************************
internal sealed class WsConnection {
    // The active WebSocket client connection.
    public readonly ClientWebSocket Socket;

    // A thread-safe FIFO queue that stores complete received text messages until read.
    public readonly ConcurrentQueue<string> Inbox = new();

    // Signals cancellation to the background receive loop when tearing down the connection.
    private readonly CancellationTokenSource _cts = new();


    //*************************************************************************************************
    // Initializes new instnace of weConnection with an existing socket connection
    // socket = established or configured socket instancce
    //*************************************************************************************************
    public WsConnection(ClientWebSocket socket) => Socket = socket;


    //*************************************************************************************************
    // Spawns asynchronous reveive loop in background thread pool
    // name = identifier for websocket conntion for logs
    //*************************************************************************************************
    public void StartReceiveLoop(string name) =>
        // Discard the Task handle ('_ =') to run the loop fire-and-forget in the background.
        _ = Task.Run(() => ReceiveLoopAsync(name));

    //*************************************************************************************************
    // Asynvhoronously listerns for incoming websocket data frames, reassembles chunked messages, and 
    // adds it to tail of completed text frames of Inbox
    // name = connection identifer for logging failures
    // return = Task representing the execution of receive loop
    //*************************************************************************************************
    private async Task ReceiveLoopAsync(string name) {
        byte[] buffer = new byte[8192];

        var messageBuffer = new List<byte>();

        try {
            while (Socket.State == WebSocketState.Open && !_cts.IsCancellationRequested) {
                WebSocketReceiveResult result = await Socket.ReceiveAsync(new ArraySegment<byte>(buffer), _cts.Token)
                    .ConfigureAwait(false);

                if (result.MessageType == WebSocketMessageType.Close)
                    break;

                messageBuffer.AddRange(new ArraySegment<byte>(buffer, 0, result.Count));

                if (result.EndOfMessage) {
                    string text = Encoding.UTF8.GetString(messageBuffer.ToArray());

                    Inbox.Enqueue(text);

                    messageBuffer.Clear();
                }
            }
        }
        catch (OperationCanceledException) {
            // Suppress exception; this is expected behavior when Close() cancels _cts.
        }
        catch (Exception ex) {
            DebugLog($"Websocket '{name}' receive loop ended: {ex.Message}");
        }
    }

    //*************************************************************************************************
    // Signals to background loop to stop, initiates a close handshake with remote websocket and
    // disposes of socket resources
    //*************************************************************************************************
    public void Close() {
        // Cancel the CancellationTokenSource to interrupt active ReceiveAsync tasks.
        _cts.Cancel();

        try {
            if (Socket.State == WebSocketState.Open) {
                Socket.CloseAsync(WebSocketCloseStatus.NormalClosure, "closed by plugin", CancellationToken.None)
                    .GetAwaiter().GetResult();
            }
        }
        catch {
            // Ignore any socket failures or timeouts during disconnect since cleanup is mandatory.
        }

        // Release all unmanaged resources held by the ClientWebSocket instance.
        Socket.Dispose();
    }
}