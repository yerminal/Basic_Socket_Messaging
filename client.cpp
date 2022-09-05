#include <iostream>
#include <algorithm>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <chrono>
#include <thread>

#define SERVER_PORT 8080
#define SIZE_BUFFER 256

void copy_string_to_vector(std::string& input, std::vector<char>& target){
    if(target.size() < input.size()){
        std::cerr << "The string must not be bigger than the vector.\n";
        exit(EXIT_FAILURE);
    }
    std::copy(input.begin(), input.end(), target.begin());
}

void reset_char_vector(std::vector<char>& v){
    std::fill(v.begin(), v.end(), 0);
}

int main(void){

    int client_fd, comm_fd;
    struct sockaddr_in server_address;
    
    int len_client_address = sizeof(struct sockaddr_in), 
        len_server_address = sizeof(struct sockaddr_in);

    int sizeof_received = 0, sizeof_sent = 0;

    std::vector<char> receive_buffer(SIZE_BUFFER, 0);
    std::vector<char> send_buffer(SIZE_BUFFER, 0);

    std::string input;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address or the address is not supported.\n";
        exit(EXIT_FAILURE);
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    std::cout << "Connecting to the server\n";
    if((comm_fd = connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address))) < 0) {
        std::cerr << "ERR: " << errno << ". " << "Connecting to the server is failed.\n";
        exit(EXIT_FAILURE);
    }

    while(1){
        std::cout << "Type a message: ";
        std::getline(std::cin, input);
        std::cout << "\n";

        if(input.size() != 0){
            copy_string_to_vector(input, send_buffer);
            
            sizeof_sent = send(client_fd, &send_buffer[0], send_buffer.size(), 0);
            if(sizeof_sent < 0){
                std::cerr << "ERR: " << errno << ". " << "send is failed.";
                exit(EXIT_FAILURE);
            }
        }
        
        reset_char_vector(send_buffer);
        // TODO: Use timeout for recv function or use threading to process recv explicitly.
        sizeof_received = recv(client_fd, &receive_buffer[0], receive_buffer.size(), 0);
        if(sizeof_received < 0){
                std::cerr << "ERR: " << errno << ". " << "recv is failed.";
                exit(EXIT_FAILURE);
        }
        if(!std::all_of(receive_buffer.begin(), receive_buffer.end(), [](int i) {return i==0;})){
            std::cout << "The received message: ";
            for (int i = 0; i < receive_buffer.size(); i++) {
                    std::cout << receive_buffer[i];
            }
            std::cout << "\n";
        }

        reset_char_vector(receive_buffer);
    }
    return 0;
}
