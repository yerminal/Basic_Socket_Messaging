#include <iostream>
#include <algorithm>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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

// #define SERVER_PORT 12396
#define SIZE_BUFFER 256

#define ERR_TIMEOUT 11

void send_message(bool *exit_system, int *client_fd, std::vector<char> *send_buffer);
void receive_message(bool *exit_system, int *client_fd, std::vector<char> *receive_buffer);
void copy_string_to_vector(std::string &input, std::vector<char> &target);
void reset_char_vector(std::vector<char> &v);
std::string copy_vector_to_string(std::vector<char> &v);
void remove_zeros_string(std::string &str);
void lookup_host(const char *host, char *hostaddrstr);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "\nUsage: " << argv[0] << " hostname port\n";
        exit(EXIT_FAILURE);
    }

    bool exit_system = 0;

    char hostaddrstr[100];

    int client_fd, comm_fd;
    struct sockaddr_in server_address;

    const std::string hostname = argv[1];
    const std::string portstr = argv[2];

    const int host_port = strtol(portstr.c_str(), NULL, 10);

    int len_client_address = sizeof(struct sockaddr_in),
        len_server_address = sizeof(struct sockaddr_in);

    std::vector<char> receive_buffer(SIZE_BUFFER, 0);
    std::vector<char> send_buffer(SIZE_BUFFER, 0);

    std::string username;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(host_port);

    lookup_host(hostname.c_str(), hostaddrstr);

    if (inet_pton(AF_INET, hostaddrstr, &server_address.sin_addr) <= 0)
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

void lookup_host(const char *hostname, char *hostaddrstr)
{
    int err;
    struct addrinfo hints, *result;
    void *ptr;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ALL;
    hints.ai_protocol = IPPROTO_TCP; /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if ((err = getaddrinfo(hostname, NULL, &hints, &result)) != 0)
    {
        std::cerr << "ERR: " << err << ". "
                  << "getaddrinfo is failed.\n";
        exit(EXIT_FAILURE);
    }

    while (result)
    {
        inet_ntop(result->ai_family, result->ai_addr->sa_data, hostaddrstr, 100);

        switch (result->ai_family)
        {
        case AF_INET:
            ptr = &((struct sockaddr_in *)result->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *)result->ai_addr)->sin6_addr;
            break;
        }
        inet_ntop(result->ai_family, ptr, hostaddrstr, 100);
        printf("IPv%d address: %s\n", result->ai_family == PF_INET6 ? 6 : 4,
               hostaddrstr);
        result = result->ai_next;
    }
    freeaddrinfo(result);
}
