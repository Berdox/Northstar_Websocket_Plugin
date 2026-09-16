import asyncio
import json
import websockets

HOST = "127.0.0.1"
PORT = 8080


async def handle_client(websocket):
    print(f"\n[+] Client connected from {websocket.remote_address}")

    try:
        async for message in websocket:
            print(f"[<] Received: {message}")

            # Prepare an echo/acknowledgment payload
            response = json.dumps(
                {
                    "status": "ok",
                    "echo": message,
                    "server_time": asyncio.get_event_loop().time(),
                }
            )

            await websocket.send(response)
            print(f"[>] Broadcast response: {response}")

    except websockets.ConnectionClosed:
        print("[-] Client disconnected")


async def main():
    print(f"[*] Starting WebSocket Server on ws://{HOST}:{PORT}")
    async with websockets.serve(handle_client, HOST, PORT):
        await asyncio.Future()  # Run forever


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n[*] Server stopped.")