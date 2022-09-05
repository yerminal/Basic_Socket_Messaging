# Basic_Socket_Messaging
## compile:
```bash
g++ server.cpp -o server & g++ client.cpp -lpthread -o client
``` 
## run:
**You can run up to 3 clients at a time. If you want to increase the max client, you should alter the macro "MAX_CLIENT".**
```bash
./server
./client
```
If you have a "Binding is failed." error, try to run
```bash
kill $(lsof -t -i:8080)
```
