# Basic_Socket_Messaging
## TODO:
- In client.cpp, Use timeout for recv function or use threading to process recv explicitly.
## compile:
```bash
g++ server.cpp -o server & g++ client.cpp -o client
``` 
## run:
```bash
./server
./client
```
If you have a "Binding is failed." error, try to run
```bash
kill $(lsof -t -i:8080)
```
