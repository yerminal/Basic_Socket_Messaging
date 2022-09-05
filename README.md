# Basic_Socket_Messaging
## compile:
- g++ server.cpp -o server & g++ client.cpp -o client
## run:
- ./server
- ./client

If you have a "Binding is failed." error, try to run
- kill $(lsof -t -i:8080)
