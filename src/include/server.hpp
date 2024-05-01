#ifndef REMOTESORT_SERVER_HPP
#define REMOTESORT_SERVER_HPP

#include <atomic>
#include <socket.hpp>
#include <util.hpp>

#include <string>
#include <vector>
#include <thread>
#include <atomic>

class Server
{
private:
    const int WAITING_REQUESTS = 5;

    std::string hostname;
    std::string port;

    Socket serverSocket;

    std::thread mainloopThread;
    std::atomic<bool> running;
public:
    Server(const char* port);
    ~Server();

    int start();
    void shutdown();

    Server(const Server& other) = delete;
    Server& operator=(const Server& other) = delete;
private:
    void mainloop();
    void sortServe(Socket &clientSocket);
    std::vector<FileInfo> sortFiles(char* path, SortBy sortBy, SortingResult *result);
    void sendResponseSuccess(Socket &clientSocket, std::vector<FileInfo> &foundFiles);
    void sendResponseFailure(Socket &clientSocket, SortingResult result);
};

#endif