# Basic_Socket_Messaging
## prerequisites
If you have gcc/g++, skip to the compile step.
- Install gcc/g++.
```bash
sudo apt update
sudo apt install build-essential
```
```bash
g++ --version
```
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
Usage: `./server port`

If you want the port automatically selected, enter `port` 0.

You can run up to 3 clients at a time. If you want to increase the max client, you should alter the macro `MAX_CLIENT` in server.cpp.

If you have `Binding server address is failed.` error, try to run:
```bash
kill $(lsof -t -i:8080)
```
### client:
- Start client with localhost address and the server's port.
```bash
./client localhost 8080
```
Usage: `./client hostname port`

Type `exit` to terminate the client.
