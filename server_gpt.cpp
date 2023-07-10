#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

int main() {
    int serverSocket;
    std::vector<int> clientSockets;
    fd_set readSet, tempSet;

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(8000);

    // Bind server socket to address
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        std::cerr << "Failed to listen." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Initialize the file descriptor sets
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);

    int maxFd = serverSocket;
    char buffer[BUFFER_SIZE];

    while (true) {
        // Make a temporary copy of the readSet to avoid modification by select()
        tempSet = readSet;

        // Call select() to monitor file descriptors for events
        if (select(maxFd + 1, &tempSet, nullptr, nullptr, nullptr) == -1) {
            std::cerr << "Error in select()." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Check if the server socket has a new connection request
        if (FD_ISSET(serverSocket, &tempSet)) {
            // Accept the new connection
            sockaddr_in clientAddress;
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress),
                                      &clientAddressLength);

            if (clientSocket == -1) {
                std::cerr << "Failed to accept client connection." << std::endl;
                close(serverSocket);
                return 1;
            }

            // Add the new client socket to the readSet and clientSockets vector
            FD_SET(clientSocket, &readSet);
            clientSockets.push_back(clientSocket);

            // Update the maxFd if necessary
            if (clientSocket > maxFd) {
                maxFd = clientSocket;
            }

            std::cout << "New client connected. Client socket: " << clientSocket << std::endl;
        }

        // Check for I/O events on client sockets
        for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ) {
            int clientSocket = *it;

            if (FD_ISSET(clientSocket, &tempSet)) {
                // Receive data from the client
                int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

                if (bytesRead == -1) {
                    std::cerr << "Error in recv()." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }

                if (bytesRead == 0) {
                    // Connection closed by client
                    std::cout << "Client socket " << clientSocket << " disconnected." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }

                // Process the received data
                buffer[bytesRead] = '\0';
                std::cout << "Received data from client socket " << clientSocket << ": " << buffer << std::endl;

                // Echo the received data back to the client
                if (send(clientSocket, buffer, bytesRead, 0) == -1) {
                    std::cerr << "Error in send()." << std::endl;
                    close(clientSocket);
                    it = clientSockets.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }

    // Close client sockets
    // for (int clientSocket : clientSockets) {
    //     close(clientSocket);
    // }

    // Close server socket
    close(serverSocket);

    return 0;
}
