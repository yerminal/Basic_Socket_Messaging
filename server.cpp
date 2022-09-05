#include <iostream>
#include <algorithm>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

// #include <chrono>
// #include <thread>

#define SERVER_PORT 8080
#define MAX_CLIENTS 3
#define SIZE_BUFFER 256


void reset_char_vector(std::vector<char>& v){
    std::fill(v.begin(), v.end(), 0);
}

int main(void){

    int server_fd, comm_fd, opt = 1, client_socket[MAX_CLIENTS] = {0};
    int select_fd, max_sd, activity;
    struct sockaddr_in server_address, client_address;
    
    int len_server_address = sizeof(struct sockaddr_in), 
        len_client_address = sizeof(struct sockaddr_in);

    int sizeof_received = 0, sizeof_sent = 0;

    std::vector<char> receive_buffer(SIZE_BUFFER, 0);
    std::vector<char> send_buffer(SIZE_BUFFER, 0);

    //set of socket descriptors 
    fd_set readfds;

    // char receive_buffer[SIZE_RECEIVE_BUFFER] = {0};
    // char send_buffer[SIZE_SEND_BUFFER] = {0};

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0){
        std::cerr << "ERR: " << errno << ". " << "setsockopt is failed.";  
        exit(EXIT_FAILURE);  
    }

    if(server_fd == -1){
        std::cerr << "ERR: " << errno << ". " << "Creating the server socket is failed.";
        exit(EXIT_FAILURE);
    }

    if(bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        std::cerr << "ERR: " << errno << ". " << "Binding server address is failed.";
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, MAX_CLIENTS) < 0){
        std::cerr << "ERR: " << errno << ". " << "Listening is failed.";
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Waiting for incoming connections...\n";

    while(1){
        FD_ZERO(&readfds);  
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)  
        {  
            select_fd = client_socket[i];

            if(select_fd > 0)  
                FD_SET(select_fd , &readfds);

            if(select_fd > max_sd)  
                max_sd = select_fd;  
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno!=EINTR))
            std::cerr << "ERR: " << errno << ". " << "select is failed.";

        if (FD_ISSET(server_fd, &readfds))  
        {  
            if((comm_fd = accept(server_fd, (struct sockaddr *)&client_address,
                (socklen_t*)&len_client_address)) < 0)
            {  
                std::cerr << "ERR: " << errno << ". " << "Accepting the client is failed.";
                exit(EXIT_FAILURE);
            }
            
            std::cout << "___New Connection___\n";
            std::cout << "socket_fd: " << comm_fd << ", ";
            std::cout << "IP: " << inet_ntoa(client_address.sin_addr) << ", ";
            std::cout << "Port: " << ntohs(client_address.sin_port) << "\n";
           
            // //send new connection greeting message 
            // if( send(new_socket, message, strlen(message), 0) != strlen(message) )  
            // {  
            //     perror("send");  
            // }  
                 
            // puts("Welcome message sent successfully");  

            for(int i = 0; i < MAX_CLIENTS; i++)  
            {  
                if(client_socket[i] == 0)  
                {
                    client_socket[i] = comm_fd;  
                    std::cout << "Adding the client to socket list in order " << i << ".\n";  
                    break;
                }  
            }  
        }

        for (int i = 0; i < MAX_CLIENTS; i++)  
        {  
            select_fd = client_socket[i];  
                 
            if(FD_ISSET(select_fd, &readfds))  
            {
                if((sizeof_received = recv(select_fd, &receive_buffer[0],
                    receive_buffer.size(), 0)) == 0)  
                {
                    getpeername(select_fd, (struct sockaddr*)&client_address,
                        (socklen_t*)&len_client_address);

                    std::cout << "___Disconnection___\n";
                    std::cout << "IP: " << inet_ntoa(client_address.sin_addr) << ", ";
                    std::cout << "Port: " << ntohs(client_address.sin_port) << "\n";

                    close(select_fd);  
                    client_socket[i] = 0;  
                }

                else 
                {
                    std::cout << "The received message: ";
                    for (int x = 0; x < receive_buffer.size(); x++) {
                            std::cout << receive_buffer[x];
                    }
                    std::cout << "\n";
                    send_buffer = receive_buffer;
                    for (int x = 0; x < MAX_CLIENTS; x++){
                        if(select_fd != client_socket[x])
                            send(client_socket[x], &send_buffer[0], send_buffer.size(), 0);
                    }
                    reset_char_vector(send_buffer); 
                }
            }  
        }

        // std::cout << "Waiting for incoming connections...\n";
        // reset_char_vector(receive_buffer);

        // comm_fd = accept(server_fd, (struct sockaddr*)&client_address, 
        //     (socklen_t*)&len_client_address);
        
        // if(comm_fd < 0){
        //     std::cerr << "ERR: " << errno << ". " << "Accepting the client is failed.";
        //     exit(EXIT_FAILURE);
        // }

        // if(!std::all_of(send_buffer.begin(), send_buffer.end(), [](int i) {return i==0;})){
        //     sizeof_sent = send(comm_fd, &send_buffer[0], send_buffer.size(), 0);
        //     if(sizeof_sent < 0){
        //         std::cerr << "ERR: " << errno << ". " << "send is failed.";
        //         exit(EXIT_FAILURE);
        //     }
        // }

        // reset_char_vector(receive_buffer);

        // sizeof_received = recv(comm_fd, &receive_buffer[0], receive_buffer.size(), 0);
        // if(sizeof_received < 0){
        //     std::cerr << "ERR: " << errno << ". " << "recv is failed.";
        //     exit(EXIT_FAILURE);
        // }

        // std::cout << "The received message: ";
        // for (int i = 0; i < receive_buffer.size(); i++) {
        //         std::cout << receive_buffer[i];
        // }
        // std::cout << "\n";

        // send_buffer = receive_buffer;

        // close(comm_fd);
    }
        // std::this_thread::sleep_for(std::chrono::milliseconds(6000));

    return 0;
}
