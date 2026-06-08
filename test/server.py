import asyncio
import urllib.parse
from websockets.server import serve

async def echo_handler(websocket):
    # Extract the URL path from the connection request
    parsed_path = urllib.parse.urlparse(websocket.path)
    
    # Check if the client is connecting to the correct endpoint
    if parsed_path.path != "/tf2":
        print(f"Rejected connection to invalid path: {parsed_path.path}")
        await websocket.close(code=4004, reason="Invalid path")
        return

    print(f"Client successfully connected to /tf2 from {websocket.remote_address}")
    
    try:
        # Keep listening for messages from this client
        async for message in websocket:
            print(f"Received from /tf2: {message}")
            
            # Echo the data back to the client
            await websocket.send(message)
            print(f"Echoed back: {message}")
            
    except Exception as e:
        print(f"Connection error or closed: {e}")
    finally:
        print("Client disconnected.")

async def main():
    print("Starting WebSocket server on ws://localhost:9001/tf2...")
    # Bind to localhost on port 9001
    async with serve(echo_handler, "localhost", 9001):
        await asyncio.Future()  # Keep the server running indefinitely

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nServer stopped manually.")