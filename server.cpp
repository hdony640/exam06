/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hdony <hdony@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/06 14:21:00 by hdony             #+#    #+#             */
/*   Updated: 2023/07/06 17:09:53 by hdony            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX_CLIENTS 128

const int BUFF_SIZE = 1024;
std::string data;
char    buff[BUFF_SIZE];

void handle_request(int fd)
{
    // std::cout << "request received\n";
    read(fd, buff, BUFF_SIZE);
    data.append(buff);
    // std::cout << "1\n";
}

/*when the server receive a message, it must resend it to all the other client with "client %d: " before every line!*/
void send_response(int fd, std::string data, int client_id)
{
    data.copy(buff, BUFF_SIZE + 1);
    buff[data.size()] = '\0';
    std::cout << "buff: " << buff;
    send(fd, buff, BUFF_SIZE, 0);
}

int main(int ac, char *argv[])
{
    int server_fd, port, ret, max_fd, new_socket, addrlen, client_id = -1;
    const int opt = 1;
    struct sockaddr_in  addr, peer_addr;
    fd_set  read_set, write_set;

    if (ac != 2)
    {
        std::cerr << "Wrong number of arguments\n";
        return (1);
    }
    port = atoi(argv[1]);
    // std::cout << "port: " << port << std::endl;
    /*the server first establishes (binds) an addr that clients can use to find the server
    **bind() assigns the addr specified by addr to the socket referred to by the file descriptor sockfd.
    */
    if ( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        std::cerr << "Fatal error (socket)\n";
        return (1);
    }
    // std::cout << "server_fd: " << server_fd << std::endl;
    if ( (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0 )
    {
        std::cerr << "Fatal error (setsockopt)\n";
        perror("setsockopt");
        return (1);
    }
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addrlen = sizeof(addr);
    if ( (ret = bind(server_fd, (struct sockaddr*)&addr, addrlen)) < 0 )
    {
        std::cerr << "Fatal error (bind)\n";
        perror("bind");
        return (1);
    }
    if ( (ret = listen(server_fd, 64)) < 0 )
    {
        std::cerr << "Fatal error (listen)\n";
        return (1);
    }
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(server_fd, &read_set);
    max_fd = server_fd;
    while (true)
    {
        fd_set  read_set_cpy = read_set;
        fd_set  write_set_cpy = write_set;
        if ( (ret = select(max_fd + 1, &read_set_cpy , &write_set_cpy, NULL, NULL)) < 0)
        {
            std::cerr << "Fatal error (select)\n";
            perror("select");
            return (1);
        }
        //there is a event on the server_fd and I need to check what to do w. the event on the socket before taking actions
        if (FD_ISSET(server_fd, &read_set_cpy))
        {
            //if there is a reading event on server_fd, client is trying to connect, accept connection
            if ( (new_socket = accept(server_fd, (struct sockaddr*)&addr, (socklen_t*)&addrlen)) < 0 )
            {
                std::cerr << "Fatal error (accept)\n";
                perror("accept");
                return (1);
            }
            ++client_id;
            printf("server: client %d just arrived\n", client_id);
            FD_SET(new_socket, &read_set);
            max_fd = new_socket;
            // std::cout << "new_socket: " << new_socket << std::endl;
            // std::cout << "File descriptors in the set: ";
            // for (int fd = 0; fd < FD_SETSIZE; ++fd) {
            // if (FD_ISSET(fd, &read_set)) {
            //     std::cout << fd << " ";
            // }
        }
        for (int i = 0; i <= max_fd; ++i)
        {
            //loop sur les les fd, check si dans read pool sauf server_fd
            // std::cout << "fd: " << i << std::endl;
            if (i == server_fd)
            {
               continue;
            }
            if (FD_ISSET(i, &read_set))
            {
                std::cout << "client socket: " << i << std::endl;
                //si reading event on a socket, there is a client request
                handle_request(i);
                std::cout << "request: " << data;
                FD_SET(i, &write_set);
                // std::cout << "2\n";
                // std::cout << "send response\n";
                send_response(i, data, client_id);
            }
        }
        //select() (non-blocking)
        //webserv can be blocking
        //accept client connection
        //read request
        //send response
    }
}