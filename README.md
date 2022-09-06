# Basic_Socket_Messaging with ngrok
## compile:
```bash
g++ server.cpp -o server & g++ client.cpp -lpthread -o client
``` 
## run:
### server:
- Start a server with a port number.
```bash
./server 8080
```
Usage: ./server port

If you want the port automatically selected, enter port 0.

You can run up to 3 clients at a time. If you want to increase the max client, you should alter the macro "MAX_CLIENT" in server.cpp.

If you have a "Binding is failed." error, try to run**
```bash
kill $(lsof -t -i:8080)
```
- Start ngrok with your server's port number.
```bash
ngrok tcp 8080
```
### client:
- Copy your ngrok address and port (example: **4.tcp.eu.ngrok.io** **12227**) (do not copy all the address tcp://4.tcp.eu.ngrok.io:12227)

- Start client with the ngrok address and port
```bash
./client 4.tcp.eu.ngrok.io 12227
```
Usage: ./client hostname port

Type "exit" to terminate the client.
