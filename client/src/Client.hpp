#pragma once
#include <string>

class Client
{
public:
    Client(const std::string &serverIP, int serverPort);
    void connect();
    void disconnect();
    void sendRequest(const std::string &request);

private:
    std::string serverIP;
    int serverPort;
    int clientSocket;
};