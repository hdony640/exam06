/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hdony <hdony@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/06 14:21:00 by hdony             #+#    #+#             */
/*   Updated: 2023/07/10 16:48:27 by hdony            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <map>

#define MAX_CLIENTS 128
#define BUFF_SIZE 1024

int                 ret;
char                buff[BUFF_SIZE];
char                msg[BUFF_SIZE];
std::map<int, int>  client;
fd_set              read_set, write_set;

void    std_error(std::string err_msg, int fd)
{
    std::cerr << err_msg;
    close(fd);
    exit (1);
}

void    send_to_all_clients(int client_sock)
{
    for (std::map<int, int>::iterator it = client.begin(); it != client.end(); ++it)
    {
        if (it->first == client_sock)
            continue;
        std::cout << "\033[1;34mrequest is resent by server to client " << it->second << " on: " << it->first << "\033[0m\n";
        send(it->first, buff, BUFF_SIZE, 0);
    }
}

void handle_request(int client_sock, int client_id)
{
    std::cout << "\033[1;32mrequest from client " << client_id << " on: " << client_sock << "\033[0m\n";
    ret = read(client_sock, buff, BUFF_SIZE);
    if (ret <= 0)
    {
        //connection closed by client
        if ( (ret = sprintf(buff, "server: client %d just left\n", client_id)) < 0)
            std_error("sprintf", -1);       
        send_to_all_clients(client_sock);
        close(client_sock);
        FD_CLR(client_sock, &read_set);
        FD_CLR(client_sock, &write_set);
        std::cout << "\033[1;35mconnection of client " << client_id << " closed on: " << client_sock << "\033[0m\n";
    }
    else
    {
        buff[ret] = '\0';
    }
}

/*when the server receive a message, it must resend it to all the other client with "client %d: " before every line!*/
void send_response(int client_sock, int client_id)
{   
    for (std::map<int, int>::iterator it = client.begin(); it != client.end(); ++it)
    {
        if (it->first == client_sock)
            continue;
        if ( (ret = sprintf(msg, "client %d: %s", it->second, buff)) < 0)
            std_error("sprintf", -1); 
        std::cout << "\033[1;34mrequest is resent by server to client " << it->second << " on: " << it->first << "\033[0m\n";
        send(it->first, msg, BUFF_SIZE, 0);
    }
}

int main(int ac, char *argv[])
{
    int                 server_fd, port, max_fd, new_socket, addrlen, peer_addr_len, client_id = -1;
    const int           opt = 1;
    struct sockaddr_in  addr, peer_addr;

    if (ac != 2)
    {
        std::cerr << "Wrong number of arguments\n";
        exit (1);
    }
    port = atoi(argv[1]);
    
    /*the server first establishes (binds) an addr that clients can use to find the server
    **bind() assigns the addr specified by addr to the socket referred to by the file descriptor sockfd.
    */
    if ( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        std_error("Fatal error\n", server_fd);
    
    if ( (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0 )
        std_error("Fatal error\n", server_fd);
    
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addrlen = sizeof(addr);
    if ( (ret = bind(server_fd, (struct sockaddr*)&addr, addrlen)) < 0 )
        std_error("Fatal error\n", server_fd);
    
    if ( (ret = listen(server_fd, MAX_CLIENTS)) < 0 )
        std_error("Fatal error\n", server_fd);
    
    // Initialize the file descriptor sets
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(server_fd, &read_set);
    max_fd = server_fd;
    
    while (true)
    {
        // Make a temporary copy of the readSet to avoid modification by select()
        fd_set  read_set_cpy = read_set;
        fd_set  write_set_cpy = write_set;
         
        // Call select() to monitor file descriptors for events
        if ( (ret = select(max_fd + 1, &read_set_cpy , &write_set_cpy, NULL, NULL)) < 0)
            std_error("select", server_fd);
        
        //there is a event on the server_fd and I need to check what to do w. the event on the socket before taking actions
        if (FD_ISSET(server_fd, &read_set_cpy))
        {
            //if there is a reading event on server_fd, client is trying to connect, accept connection
            peer_addr_len = sizeof(peer_addr);
            if ( (new_socket = accept(server_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&peer_addr_len)) < 0 )
                std_error("accept", new_socket);
            ++client_id;
            
            // Add the new client socket to the readSet and clientSockets vector
            FD_SET(new_socket, &read_set);
            client.insert(std::pair<int, int>(new_socket, client_id));
            max_fd = new_socket;
            if ( (ret = sprintf(buff, "server: client %d just arrived\n", client_id)) < 0)
                std_error("sprintf", -1);
            send(new_socket, buff, BUFF_SIZE, 0);
            std::cout << "\033[1;31mclient " << client_id << " connection accepted on: " << new_socket << "\033[0m\n";
        }
        
        // Check for I/O events on client sockets
        for (std::map<int, int>::iterator it = client.begin(); it != client.end(); it++)
        {
            if (FD_ISSET(it->first, &read_set_cpy))
            {
                //si reading event on a socket, there is a client request
                handle_request(it->first, it->second);
                FD_SET(it->first, &write_set);
                send_response(it->first, it->second);
            }
        }
        //select() (non-blocking)
        //webserv can be blocking
    }
}