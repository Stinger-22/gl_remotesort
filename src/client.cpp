#include <client.hpp>
#include <util.hpp>
#include <socket.hpp>

#include <cstring>
#include <iostream>
#include <unistd.h>

Client::Client(const char* serverHostname, const char* port): serverHostname(serverHostname), port(port) {};

void Client::request()
{
    clientSocket = std::move(Socket(serverHostname.c_str(), port.c_str()));
    if (clientSocket.getState() == Socket::State::FAILED)
    {
        return;
    }
    if (clientSocket.connect() == -1)
    {
        return;
    }

    askSorting();
    receiveAnswer();
}

void Client::askSorting()
{
    int messageSize;
    char buffer[4096] = {0};
    std::cout << "Please enter path on server where sort files: ";
    std::cin.getline(buffer + sizeof(char), 4095 - sizeof(char));
    std::cout << "Please enter sort type (NAME = 1, TYPE = 2, DATE = 3): ";
    int sortType;
    std::cin >> sortType;
    std::memcpy(buffer, (void*) (&sortType), sizeof(char));
    messageSize = clientSocket.write(buffer, strlen(buffer));
    if (messageSize < 0)
    {
        std::clog << "[Error] Failed to write message to server: write(): " << std::system_category().message(errno) << std::endl;
        return;
    }
}

void Client::receiveAnswer()
{
    int messageSize;
    char buffer[4096] = {0};
    int numberOfFiles;
    messageSize = clientSocket.read(&numberOfFiles, sizeof(int));
    if (messageSize < 0)
    {
        std::clog << "[Error] Failed to read server's reply: read(): " << std::system_category().message(errno) << std::endl;
    }

    if (numberOfFiles < 0)
    {
        std::cout << "Server failed to sort files" << std::endl;
        switch (numberOfFiles)
        {
            case int(SortingResult::FAILURE_PATH_IS_NOT_DIRECTORY):
                std::cout << "You typed path which is not a directory." << std::endl;
                break;
            case int(SortingResult::FAILURE_WRONG_SORT_TYPE):
                std::cout << "You typed unknown sorting type." << std::endl;
                break;
            default:
                std::cout << "Unknown error." << std::endl;
        }
        return;
    }

    std::cout << "Total number of files in folder: " << numberOfFiles << std::endl;
    for (int i = 0; i < numberOfFiles; i++)
    {
        std::memset(buffer, 0, sizeof(buffer));
        messageSize = clientSocket.read(buffer, 4095);
        if (messageSize < 0)
        {
            std::clog << "[Error] Failed to read server's reply: read(): " << std::system_category().message(errno) << std::endl;
        }
        std::cout << "Received: " << buffer << std::endl;
    }
}