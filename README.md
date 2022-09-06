# Basic_Socket_Messaging with ngrok
## prerequisites
If you have these packages, skip to the compile step.
- Install gcc/g++.
```bash
sudo apt update
sudo apt install build-essential
```
```bash
g++ --version
```
- Install ngrok. (for more details, https://ngrok.com/docs/getting-started).
### 
```bash
curl -s https://ngrok-agent.s3.amazonaws.com/ngrok.asc | \
      sudo tee /etc/apt/trusted.gpg.d/ngrok.asc >/dev/null && \
      echo "deb https://ngrok-agent.s3.amazonaws.com buster main" | \
      sudo tee /etc/apt/sources.list.d/ngrok.list && \
      sudo apt update && sudo apt install ngrok
```
## compile:
```bash
g++ server.cpp -o server && g++ client.cpp -lpthread -o client
``` 
## run:
### server:
- Start a server with a port number.
```bash
./server 8080
```
Usage: `./server port`

If you want the port automatically selected, enter `port` 0.

You can run up to 3 clients at a time. If you want to increase the max client, you should alter the macro `MAX_CLIENT` in server.cpp.

If you have `Binding server address is failed.` error, try to run:
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
Usage: `./client hostname port`

Type `exit` to terminate the client.
