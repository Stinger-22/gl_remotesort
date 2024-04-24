#ifndef REMOTESORT_SERVER_HPP
#define REMOTESORT_SERVER_HPP

#include <util.hpp>

#include <string>
#include <vector>
#include <netdb.h>

class Server
{
private:
    int WAITING_REQUESTS = 5;

    std::string hostname;
    std::string port;

    int serverSocket;

public:
    Server(const char* port);
    ~Server();

    int start();
    void shutdown();

    Server(const Server& other) = delete;
    Server& operator=(const Server& other) = delete;
private:
    addrinfo* getServerAddresses() noexcept;
    int createServerSocket(addrinfo* serverAddress) noexcept;

    void mainloop();
    void echoServe(int clientSocket);
    void sortServe(int clientSocket);
    std::vector<struct FileInfo> sortFiles(char* path, SortBy sortBy, SortingResult *result);
    void sendResponseSuccess(int clientSocket, std::vector<struct FileInfo> foundFiles);
    void sendResponseFailure(int clientSocket, SortingResult result);
};

#endif