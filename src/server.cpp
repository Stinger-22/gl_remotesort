#include <server.hpp>
#include <networking.hpp>
#include <string>
#include <util.hpp>

#include <algorithm>
#include <filesystem>
#include <cerrno>
#include <iostream>
#include <climits>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <sys/stat.h>

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

    // WARNING: User have to remember to free this. Maybe should create wrapper class
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
        sortServe(clientSocket);
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

void Server::sortServe(int clientSocket)
{
    int messageSize;
    // BUFFER STRUCTURE: | CHAR - type of sorting | CHAR[] - path |
    char buffer[4096];
    char sortType;
    std::memset(buffer, 0, sizeof(buffer));
    int readSize = 0;
    messageSize = read(clientSocket, buffer, 4095);
    std::cout << "messageSize = " << messageSize << std::endl;
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to read client's message: read(): " << std::system_category().message(errno) << std::endl;
        return;
    }

    std::memcpy((void*)(&sortType), buffer, sizeof(char));

    SortingRequest::SortBy sortBy;
    switch (sortType) {
        case 1:
            sortBy = SortingRequest::SortBy::NAME;
            break;
        case 2:
            sortBy = SortingRequest::SortBy::TYPE;
            break;
        case 3:
            sortBy = SortingRequest::SortBy::DATE;
            break;
        default:
            std::cerr << "[Error] Client send wrong sorting type." << std::endl;
            // TODO send error
    }
    std::vector<struct FileInfo> foundFiles = sortFiles(buffer + sizeof(char), sortBy);
    sendResponse(clientSocket, foundFiles);
}

std::vector<struct FileInfo> Server::sortFiles(char* path, SortingRequest::SortBy sortBy)
{
    namespace fs = std::filesystem;

    std::vector<struct FileInfo> foundFiles;
    std::string pathString = path;
    try
    {
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(pathString, fs::directory_options::skip_permission_denied))
        {
            if (entry.is_regular_file())
            {
                struct FileInfo fileInfo;
                fileInfo.filename = remove_extension(entry.path().filename());
                fileInfo.extension = entry.path().extension();
                struct stat attr;
                stat(entry.path().c_str(), &attr);
                fileInfo.time = attr.st_mtime;
                foundFiles.push_back(fileInfo);
            }
        }
    }
    catch(std::filesystem::filesystem_error& e)
    {
      // TODO failure
    }

    int (*comparators[3])(const FileInfo&, const FileInfo&);
    comparators[0] = compareByFilename;
    comparators[1] = compareByExtension;
    comparators[2] = compareByTime;

    std::sort(foundFiles.begin(), foundFiles.end(), comparators[sortBy]);
    for (size_t i = 0; i < foundFiles.size(); i++)
    {
        struct FileInfo info = foundFiles[i];
        std::cout << info.filename << " | " << info.extension << " | " << info.time << std::endl;
    }
    std::cout << '\n';
    return foundFiles;
}

void Server::sendResponse(int clientSocket, std::vector<struct FileInfo> foundFiles)
{
    int messageSize;
    int numberOfFiles = foundFiles.size();
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    messageSize = write(clientSocket, &numberOfFiles, sizeof(int));
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to send number of files: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }
    for (int i = 0; i < numberOfFiles; i++)
    {
        std::memset(buffer, 0, sizeof(buffer));
        std::string fileInfoStr;
        struct FileInfo fileInfo = foundFiles[i];
        fileInfoStr = fileInfo.filename + fileInfo.extension + " | " + std::to_string(fileInfo.time);
        messageSize = write(clientSocket, fileInfoStr.c_str(), 4095);
        if (messageSize < 0)
        {
            std::clog << "[Error] Failed to read server's reply: read(): " << std::system_category().message(errno) << std::endl;
        }
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
