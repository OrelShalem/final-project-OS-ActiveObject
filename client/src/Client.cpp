// This file implements the Client class, which handles communication with the server.

#include "Client.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// Constructor: Initializes the client with server IP and port
Client::Client(const std::string &ip, int port) : serverIP(ip), serverPort(port), clientSocket(-1) {}

// Establishes a connection to the server
void Client::connect()
{
    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        throw std::runtime_error("Error creating socket");
    }

    // Set up the server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid address / Address not supported");
    }

    // Connect to the server
    if (::connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw std::runtime_error("Connection failed");
    }

    std::cout << "Connected to server" << std::endl;
}

// Closes the connection to the server
void Client::disconnect()
{
    if (clientSocket != -1)
    {
        close(clientSocket);
        clientSocket = -1;
        std::cout << "Disconnected from server" << std::endl;
    }
}

// Sends a request to the server and receives the response
void Client::sendRequest(const std::string &request)
{
    if (clientSocket == -1)
    {
        throw std::runtime_error("Not connected to server");
    }

    // Send the request to the server
    send(clientSocket, request.c_str(), request.length(), 0);
    std::cout << "Sent request: " << request << std::endl;

    // Receive and print the server's response
    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    std::cout << "Server response: " << std::string(buffer, valread) << std::endl;
}