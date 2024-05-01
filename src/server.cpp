#include <server.hpp>
#include <socket.hpp>
#include <util.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <system_error>
#include <cerrno>
#include <vector>
#include <algorithm>

Server::Server(const char* port)
{
    this->hostname = getHostName();
    this->port = port;
}

int Server::start()
{
    this->serverSocket = std::move(Socket(hostname.c_str(), port.c_str()));
    if (serverSocket.getState() == Socket::State::FAILED)
    {
        return -1;
    }
    if (serverSocket.bind() == -1)
    {
        return -1;
    }
    if (serverSocket.listen(WAITING_REQUESTS) == -1)
    {
        return -1;
    }

    serverSocket.printHostname();
    serverSocket.printIP();
    serverSocket.printPort();
    mainloop();
    // Can't return 0 cause server is running in the same thread.
}

void Server::shutdown()
{
    std::cerr << "[Info] Shutting down the server...\n";
    serverSocket.close();
    std::cerr << "[Info] Server is shut down." << std::endl;
}


void Server::mainloop()
{
    while (1)
    {
        Socket client = serverSocket.accept();
        if (client.getState() != Socket::State::ACCEPTED)
        {
            std::clog << "[Error] Couldn't accept client socket: mainloop()." << std::endl;
        }
        std::clog << "[Info] Accepted client socket" << std::endl;
        sortServe(client);
    }
    // TODO this method should be run in a new thread
}

void Server::sortServe(Socket &clientSocket)
{
    int messageSize;
    char buffer[4096] = {0};

    messageSize = clientSocket.read(buffer, 4095);
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to read client's message: read(): " << std::system_category().message(errno) << std::endl;
        return;
    }

    SortBy sortBy;
    std::memcpy((void*)(&sortBy), buffer, sizeof(char));
    sortBy = SortBy(int(sortBy) - 1); // TODO fix this
    if (int(sortBy) < 0 || int(sortBy) > 2)
    {
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

std::vector<FileInfo> Server::sortFiles(char* path, SortBy sortBy, SortingResult *result)
{
    std::clog << "[Info] Sorting files in: " << path << " by " << int(sortBy) << std::endl;
    namespace fs = std::filesystem;
    std::vector<FileInfo> foundFiles;
    std::string_view pathString = path;
    if (fs::is_directory(path) == false)
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
                FileInfo fileInfo;
                fileInfo.filename = entry.path().stem();
                fileInfo.extension = entry.path().extension();
                fileInfo.time = getFileLastWriteTime(entry.path().c_str());
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

void Server::sendResponseSuccess(Socket &clientSocket, std::vector<FileInfo> &foundFiles)
{
    // TODO make this better
    int messageSize;
    int numberOfFiles = foundFiles.size();
    char buffer[4096] = {0};
    messageSize = clientSocket.write(&numberOfFiles, sizeof(int));
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
        messageSize = clientSocket.write((void*) fileInfoStr.c_str(), 4095);
        if (messageSize < 0)
        {
            std::clog << "[Error] Failed to read server's reply: read(): " << std::system_category().message(errno) << std::endl;
        }
    }
    std::clog << "[Info] Sent the client a response about the success." << std::endl;
}

void Server::sendResponseFailure(Socket &clientSocket, SortingResult result)
{
    int messageSize;
    messageSize = clientSocket.write(&result, sizeof(SortingResult));
    if (messageSize == -1)
    {
        std::clog << "[Error] Failed to send error: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }
    std::clog << "[Info] Sent a response to the client about the failure." << std::endl;
}
