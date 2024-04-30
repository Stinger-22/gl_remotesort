#include <server.hpp>
#include <string>
#include <system_error>
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
        std::clog << "[Info] Accepted client socket" << std::endl;
        sortServe(clientSocket);
        close(clientSocket);
    }
    // TODO serverSocket is not closed. Perhaps this method should be run in a new thread
}

void Server::sortServe(int clientSocket)
{
    int messageSize;
    char buffer[4096] = {0};
    char sortType;

    messageSize = read(clientSocket, buffer, 4095);
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to read client's message: read(): " << std::system_category().message(errno) << std::endl;
        return;
    }

    std::memcpy((void*)(&sortType), buffer, sizeof(char));
    SortBy sortBy;
    switch (sortType) {
        case 1:
            sortBy = SortBy::NAME;
            break;
        case 2:
            sortBy = SortBy::TYPE;
            break;
        case 3:
            sortBy = SortBy::DATE;
            break;
        default:
            std::clog << "[Error] Client send wrong sorting type." << std::endl;
            sendResponseFailure(clientSocket, SortingResult::FAILURE_WRONG_SORT_TYPE);
            return;
    }

    SortingResult result;
    std::vector<struct FileInfo> foundFiles = sortFiles(buffer + sizeof(char), sortBy, &result);
    if (result == SortingResult::SUCCESS)
    {
        sendResponseSuccess(clientSocket, foundFiles);
    }
    else
    {
        sendResponseFailure(clientSocket, result);
    }
}

std::vector<struct FileInfo> Server::sortFiles(char* path, SortBy sortBy, SortingResult *result)
{
    std::clog << "[Info] Sorting files in: " << path << " by " << int(sortBy) << std::endl;
    namespace fs = std::filesystem;
    std::vector<struct FileInfo> foundFiles;
    std::string pathString = path;
    if (std::filesystem::is_directory(path) == false)
    {
        *result = SortingResult::FAILURE_PATH_IS_NOT_DIRECTORY;
        std::clog << "[Error] Sorting files failed. Received path is not directory." << std::endl;
        return foundFiles;
    }
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
    catch (std::filesystem::filesystem_error& e)
    {
        std::clog << "[Error] sortFiles(): " << e.what() << std::endl;
    }

    extern int (*comparators[3])(const FileInfo&, const FileInfo&);
    std::sort(foundFiles.begin(), foundFiles.end(), comparators[int(sortBy)]);
    *result = SortingResult::SUCCESS;
    std::clog << "[Info] Sorting files finished." << std::endl;
    return foundFiles;
}

void Server::sendResponseSuccess(int clientSocket, std::vector<struct FileInfo> foundFiles)
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
    std::clog << "[Info] Sent the client a response about the success." << std::endl;
}

void Server::sendResponseFailure(int clientSocket, SortingResult result)
{
    int messageSize;
    messageSize = write(clientSocket, &result, sizeof(SortingResult));
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to send error: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }
    std::clog << "[Info] Sent a response to the client about the failure." << std::endl;
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
