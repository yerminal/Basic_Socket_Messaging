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
#include <sstream>
#include <iterator>

#include <chrono>
#include <thread>

#define SERVER_PORT 8080
#define SIZE_BUFFER 256

#define ERR_TIMEOUT 11

void send_message(bool *exit_system, int *client_fd, std::vector<char> *send_buffer);
void receive_message(bool *exit_system, int *client_fd, std::vector<char> *receive_buffer);
void copy_string_to_vector(std::string &input, std::vector<char> &target);
void reset_char_vector(std::vector<char> &v);
std::string copy_vector_to_string(std::vector<char> &v);
void remove_zeros_string(std::string &str);

int main(void)
{
    bool exit_system = 0;

    int client_fd, comm_fd;
    struct sockaddr_in server_address;

    int len_client_address = sizeof(struct sockaddr_in),
        len_server_address = sizeof(struct sockaddr_in);

    std::vector<char> receive_buffer(SIZE_BUFFER, 0);
    std::vector<char> send_buffer(SIZE_BUFFER, 0);

    std::string username;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        std::cerr << "Invalid address or the address is not supported.\n";
        exit(EXIT_FAILURE);
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "setsockopt is failed.";
        exit(EXIT_FAILURE);
    }

    std::cout << "Connecting to the server...\n";
    if ((comm_fd = connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "Connecting to the server is failed.\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to the server.\n";

    while (1)
    {
        if (recv(client_fd, &receive_buffer[0], receive_buffer.size(), 0) < 0)
        {
            if (errno != ERR_TIMEOUT)
            {
                std::cerr << "ERR: " << errno << ". "
                          << "recv is failed.";
                exit(EXIT_FAILURE);
            }
        }
        username = copy_vector_to_string(receive_buffer);
        if (username.find("socket_") != std::string::npos)
        {
            break;
        }
    }

    std::cout << "\nYour username: " << username << "\n";
    reset_char_vector(receive_buffer);

    std::thread th_receive_message(receive_message, &exit_system, &client_fd, &receive_buffer);
    std::thread th_send_message(send_message, &exit_system, &client_fd, &send_buffer);

    th_send_message.join();
    th_receive_message.join();

    close(comm_fd);
    close(client_fd);

    return 0;
}

void send_message(bool *exit_system, int *client_fd, std::vector<char> *send_buffer)
{
    std::string input;
    while (1)
    {
        std::cout << "Type a message: ";
        std::getline(std::cin, input);
        std::cout << "\n";

        if (input == "exit")
        {
            *exit_system = 1;
            std::cout << "Exiting...\n";
            break;
        }

        if (input.size() != 0)
        {
            remove_zeros_string(input);
            copy_string_to_vector(input, *send_buffer);

            if (send(*client_fd, &(*send_buffer)[0], (*send_buffer).size(), 0) < 0)
            {
                std::cerr << "ERR: " << errno << ". "
                          << "send is failed.";
                exit(EXIT_FAILURE);
            }
        }
        reset_char_vector(*send_buffer);
    }
}

void receive_message(bool *exit_system, int *client_fd, std::vector<char> *receive_buffer)
{
    while (!(*exit_system))
    {
        if (recv(*client_fd, &(*receive_buffer)[0], (*receive_buffer).size(), 0) < 0)
        {
            if (errno != ERR_TIMEOUT)
            {
                std::cerr << "ERR: " << errno << ". "
                          << "recv is failed.";
                exit(EXIT_FAILURE);
            }
        }
        if (!std::all_of((*receive_buffer).begin(), (*receive_buffer).end(), [](int i)
                         { return i == 0; }))
        {
            std::cout << "\n";
            std::cout << copy_vector_to_string((*receive_buffer));
            std::cout << "\n\n";
        }
        reset_char_vector((*receive_buffer));
    }
}

void reset_char_vector(std::vector<char> &v)
{
    std::fill(v.begin(), v.end(), 0);
}

void copy_string_to_vector(std::string &input, std::vector<char> &target)
{
    if (target.size() < input.size())
    {
        std::cerr << "The string must not be bigger than the vector.\n";
        exit(EXIT_FAILURE);
    }
    std::copy(input.begin(), input.end(), target.begin());
}

std::string copy_vector_to_string(std::vector<char> &v)
{
    std::ostringstream oss;
    std::copy(v.begin(), v.end(), std::ostream_iterator<char>(oss, ""));
    return oss.str();
}

void remove_zeros_string(std::string &str)
{
    str.erase(std::remove(str.begin(), str.end(), '\0'), str.end());
}
