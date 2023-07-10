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
#include <arpa/inet.h>
#include <unistd.h>

const int BUFF_SIZE = 1024;

int main(int ac, char *argv[])
{
    int client_fd, addrlen, ret, opt = 1;
    char    buff[BUFF_SIZE];
    struct sockaddr_in addr;

    if ( (client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        std::cerr << "Fatal error (socket)\n";
        return (1);
    }
    // std::cout << "client_fd: " << client_fd << std::endl;
    if ( (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0 )
    {
        std::cerr << "Fatal error (setsockopt)\n";
        perror("setsockopt");
        return (1);
    }
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addrlen = sizeof(addr);
    // if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr)
    //     <= 0) {
    //     printf(
    //         "\nInvalid address/ Address not supported \n");
    //     return -1;
    // }
    if ( (ret = connect(client_fd, (struct sockaddr*)&addr, addrlen)) < 0 )
    {
        std::cerr << "Fatal error (connect)\n";
        perror("connect");
        return (1);
    }
    // std::cout << buff;
    //send a request to the server
    while (true)
    {
        std::cout << "msg to server: ";
		bzero(buff,256);
		fgets(buff,255, stdin);
        // std::cout << buff;
        if ( (ret = send(client_fd, buff, BUFF_SIZE, 0)) < 0)
        {
            std::cerr << "Fatal error (send)\n";
            perror("send");
            return (1);
        }
        // std::cout << "request sent\n";
        //receive the answer from the server
        if ( (ret = read(client_fd, buff, BUFF_SIZE) ) < 0)
        {
            std::cerr << "Fatal error (read)\n";
            perror("read");
            return (1);
        }
        std::cout << "answer from server: " << buff;
    }
    
}