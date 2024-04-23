#include "util.hpp"
#include <client.hpp>

#include <cstring>
#include <iostream>
#include <unistd.h>

Client::~Client()
{

}

void Client::request()
{
    std::cout << "askSorting" << std::endl;

    addrinfo* serverAddress = resolveServerAddress();
    if (serverAddress == nullptr)
    {
        return;
    }

    clientSocket = createClientSocket(serverAddress);
    if (clientSocket == -1)
    {
        return;
    }

    if (connectToServer(serverAddress) == -1)
    {
        return;
    }

    // WARNING: User have to remember to free this. Maybe should create wrapper class
    freeaddrinfo(serverAddress);

    int messageSize;
    char buffer[256];
    std::cout << "Please enter the message: ";
    std::memset(buffer, 0, sizeof(buffer));
    std::cin.getline(buffer, 255);
    messageSize = write(clientSocket, buffer, strlen(buffer));
    if (messageSize < 0)
    {
        std::clog << "[Error] Failed to write message to server: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }

    std::memset(buffer, 0, sizeof(buffer));
    messageSize = read(clientSocket, buffer, 255);
    if (messageSize < 0)
    {
        std::clog << "[Error] Failed to read server's reply: read(): " << std::system_category().message(errno) << std::endl;
    }
    std::cout << "Echo: " << buffer << std::endl;
    close(clientSocket);
}

addrinfo* Client::resolveServerAddress() noexcept
{
    std::clog << "[Debug] Resolving server address.\n";
    addrinfo hints, *serverAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(serverHostname.c_str(), port.c_str(), &hints, &serverAddress) == -1)
    {
        std::clog << "[Error] Failed to resolve server address: getaddrinfo(): " << std::system_category().message(errno) << std::endl;
        return nullptr;
    }

    return serverAddress;
}

int Client::createClientSocket(addrinfo* serverAddress) noexcept
{
    if (serverAddress == nullptr)
    {
        std::clog << "[Error] createClientSocket(): serverAddress is nullptr." << std::endl;
        return -1;
    }

    std::clog << "[Debug] Creating client socket.\n";
    int clientSocket = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
    if (clientSocket == -1)
    {
        std::clog << "[Error] Failed to create client socket: socket(): " << std::system_category().message(errno) << std::endl;
        return -1;
    }
    return clientSocket;
}

int Client::connectToServer(addrinfo* serverAddress) noexcept
{
    std::clog << "[Info] Connecting to server (" << serverHostname << ":" << port << ")\n";
    if (connect(clientSocket, serverAddress->ai_addr, serverAddress->ai_addrlen) == -1)
    {
        std::clog << "[Error] Connection to server failed: connect(): " << std::system_category().message(errno) << std::endl;
        return -1;
    }
    std::clog << "[Info] Successfully connected to " << serverHostname << ":" << port << std::endl;
    return 0;
}

void Client::askSorting()
{
    std::cout << "WIP" << std::endl;
}