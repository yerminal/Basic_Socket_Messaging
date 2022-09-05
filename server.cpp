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
#include <sstream>
#include <iterator>

#define SERVER_PORT 8080
#define MAX_CLIENT 3
#define SIZE_BUFFER 256

void reset_char_vector(std::vector<char> &v);
void copy_string_to_vector(std::string &input, std::vector<char> &target);
std::string copy_vector_to_string(std::vector<char> &v);
void remove_zeros_string(std::string &str);

int main(void)
{

    int server_fd, comm_fd, opt = 1, client_socket[MAX_CLIENT] = {0};
    int select_fd, max_sd, activity;
    struct sockaddr_in server_address, client_address;

    int len_server_address = sizeof(struct sockaddr_in),
        len_client_address = sizeof(struct sockaddr_in);

    std::vector<char> receive_buffer(SIZE_BUFFER, 0);
    std::vector<char> send_buffer(SIZE_BUFFER, 0);
    std::string username, message;

    fd_set readfds;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "setsockopt is failed.";
        exit(EXIT_FAILURE);
    }

    if (server_fd == -1)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "Creating the server socket is failed.";
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "Binding server address is failed.";
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENT) < 0)
    {
        std::cerr << "ERR: " << errno << ". "
                  << "Listening is failed.";
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for incoming connections...\n";

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            select_fd = client_socket[i];

            if (select_fd > 0)
                FD_SET(select_fd, &readfds);

            if (select_fd > max_sd)
                max_sd = select_fd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            std::cerr << "ERR: " << errno << ". "
                      << "select is failed.";
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            if ((comm_fd = accept(server_fd, (struct sockaddr *)&client_address,
                                  (socklen_t *)&len_client_address)) < 0)
            {
                std::cerr << "ERR: " << errno << ". "
                          << "Accepting the client is failed.";
                exit(EXIT_FAILURE);
            }

            std::cout << "\n___New Connection___\n";
            std::cout << "socket_fd: " << comm_fd << ", ";
            std::cout << "IP: " << inet_ntoa(client_address.sin_addr) << ", ";
            std::cout << "Port: " << ntohs(client_address.sin_port) << "\n\n";

            reset_char_vector(send_buffer);
            username = "socket_" + std::to_string(comm_fd);
            remove_zeros_string(username);
            copy_string_to_vector(username, send_buffer);

            std::cout << "Username: " << username << "\n";

            if (send(comm_fd, &send_buffer[0], send_buffer.size(), 0) < 0)
            {
                std::cerr << "ERR: " << errno << ". "
                          << "send is failed.";
                exit(EXIT_FAILURE);
            }
            reset_char_vector(send_buffer);

            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = comm_fd;
                    std::cout << "Adding the client to socket list in order " << i << ".\n";
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            select_fd = client_socket[i];

            if (FD_ISSET(select_fd, &readfds))
            {
                if (recv(select_fd, &receive_buffer[0], receive_buffer.size(), 0) == 0)
                {
                    getpeername(select_fd, (struct sockaddr *)&client_address,
                                (socklen_t *)&len_client_address);

                    std::cout << "\n___Disconnection___\n";
                    std::cout << "IP: " << inet_ntoa(client_address.sin_addr) << ", ";
                    std::cout << "Port: " << ntohs(client_address.sin_port) << "\n\n";

                    close(select_fd);
                    client_socket[i] = 0;
                }

                else
                {
                    message = copy_vector_to_string(receive_buffer);
                    message = "socket_" + std::to_string(select_fd) + ": " + message;
                    remove_zeros_string(message);
                    copy_string_to_vector(message, send_buffer);

                    std::cout << message << "\n";

                    for (int x = 0; x < MAX_CLIENT; x++)
                    {
                        if (select_fd != client_socket[x] && client_socket[x] != 0)
                        {
                            if (send(client_socket[x], &send_buffer[0], send_buffer.size(), 0) < 0)
                            {
                                std::cerr << "ERR: " << errno << ". "
                                          << "send is failed.";
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    reset_char_vector(receive_buffer);
                    reset_char_vector(send_buffer);
                }
            }
        }
    }

    return 0;
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
