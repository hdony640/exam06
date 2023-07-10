#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int BUFFER_SIZE = 1024;

int main() {
    int clientSocket;
    char buffer[BUFFER_SIZE];

    // Create client socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to the server IP address
    serverAddress.sin_port = htons(8000); // Change to the server port

    // Connect to the server
    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to the server." << std::endl;
        close(clientSocket);
        return 1;
    }

    // Read requests from the user and send them to the server
    std::string request;
    while (true) {
        std::cout << "Enter a request (or 'quit' to exit): ";
        std::getline(std::cin, request);

        // Check if the user wants to quit
        if (request == "quit") {
            break;
        }

        // Send the request to the server
        if (send(clientSocket, request.c_str(), request.size(), 0) == -1) {
            std::cerr << "Error in send()." << std::endl;
            close(clientSocket);
            return 1;
        }

        // Receive response from the server
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead == -1) {
            std::cerr << "Error in recv()." << std::endl;
            close(clientSocket);
            return 1;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Response from server: " << buffer << std::endl;
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
