#include <cerrno>
#include <server.hpp>

#include <iostream>
#include <climits>
#include <cstring>
#include <unistd.h>
#include <netdb.h>


Server::Server(const char* port)
{
    this->port = port;
    char hostname[HOST_NAME_MAX + 1] = {0};
    gethostname(hostname, HOST_NAME_MAX);
    this->hostname = hostname;
}

Server::~Server()
{

}

int Server::start()
{
    addrinfo* serverAddresses = getServerAddresses();
    if (serverAddresses == nullptr)
    {
        return -1;
    }

    serverSocket = createServerSocket(serverAddresses);
    if (serverSocket == -1)
    {
        return -1;
    }

    // WARNING User have to remember to free this.
    freeaddrinfo(serverAddresses);

    mainloop();
    // Can't return 0 cause server is running in the same thread.
}

void Server::shutdown()
{
    std::cerr << "[Info] Shutting down the server...\n";
    if (close(serverSocket) == -1)
    {
        std::clog << "[Error] Failed to close serverSocket: close(): " << std::system_category().message(errno) << std::endl;
        exit(-1);
    }
    std::cerr << "[Info] Server is shut down." << std::endl;
}


void Server::mainloop()
{
    struct sockaddr_in clientAddress;
    socklen_t clientSize = sizeof(clientAddress);;
    int clientSocket;
    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientSize);
        if (clientSocket == -1)
        {
            std::clog << "[Error] Failed to accept socket: accept(): " << std::system_category().message(errno) << std::endl;
        }
        echoServe(clientSocket);
        close(clientSocket);
    }
    // serverSocket is not closed
}

void Server::echoServe(int clientSocket)
{
    int messageSize;
    char buffer[256];

    std::memset(buffer, 0, sizeof(buffer));
    // TODO Here should be reading cycle
    messageSize = read(clientSocket, buffer, 255);
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to read client's message: read(): " << std::system_category().message(errno) << std::endl;
        return;
    }
    std::cout << "Received message: " << buffer << std::endl;


    messageSize = write(clientSocket, buffer, messageSize);
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to write reply with echo: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }
}

addrinfo* Server::getServerAddresses() noexcept
{
    addrinfo hints, *serverAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::clog << "[Debug] Resolving server hostname.\n";
    if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &serverAddress) != 0)
    {
        std::clog << "[Error] Failed to resolve server's local address: getaddrinfo(): " << std::system_category().message(errno) << std::endl;
        return nullptr;
    }

    return serverAddress;
}

int Server::createServerSocket(addrinfo* serverAddress) noexcept
{
    if (serverAddress == nullptr)
    {
        std::clog << "[Error] createServerSocket(): serverAddress is nullptr." << std::endl;
        return -1;
    }

    std::clog << "[Debug] Creating server socket object.\n";
    int serverSocket = socket(serverAddress->ai_family,
                              serverAddress->ai_socktype,
                              serverAddress->ai_protocol);
    if (serverSocket == -1)
    {
        std::clog << "[Error] Failed to create server socket: socket(): " << std::system_category().message((errno)) << std::endl;
        return -1;
    }

    std::clog << "[Debug] Binding socket to the resolved address." << std::endl;
    if (bind(serverSocket, serverAddress->ai_addr, serverAddress->ai_addrlen) == -1)
    {
        std::clog << "[Error] Failed to bind server socket to address " << hostname << ":" << port << " : bind(): " << std::system_category().message((errno)) << std::endl;
        return -1;
    }

    std::clog << "[Debug] Activating server listenning mode.\n";
    if (listen(serverSocket, WAITING_REQUESTS) == -1){
        std::clog << "[Error] Failed to activate socket listenner: listen(): " << std::system_category().message((errno)) << std::endl;
        return -1;
    }
    std::clog << "[Info] Server is listenning for incoming connections at " << hostname << ":" << port << std::endl;
    return serverSocket;
}
