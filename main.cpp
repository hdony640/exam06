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

int main(int ac, char *argv[])
{
    int server_fd, port, ret;
    const int opt = 1;
    struct sockaddr_in  addr;
    fd_set  read_set, write_set;
    
    if (ac != 2)
    {
        std::cerr << "Wrong number of arguments\n";
        return (1);
    }
    port = atoi(argv[1]);
    std::cout << "port: " << port << std::endl;
    /*the server first establishes (binds) an addr that clients can use to find the server
    **bind() assigns the addr specified by addr to the socket referred to by the file descriptor sockfd.
    */
    if ( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        std::cerr << "Fatal error (socket)\n";
        return (1);
    }
    std::cout << "server_fd: " << server_fd << std::endl;
    if ( (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0 )
    {
        std::cerr << "Fatal error (setsockopt)\n";
        perror("setsockopt");
        return (1);
    }
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( (ret = bind(server_fd, (struct sockaddr*)&addr,  sizeof(addr))) < 0 )
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
    while (true)
    {
        fd_set  read_set_cpy = read_set;
        fd_set  write_set_cpy = write_set;
        int select(int nfds, , fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);
        //select() (non-blocking)
        //webserv can be blocking
        //accept client connection
        //read request
        //send response
    }
}