#ifndef REMOTESORT_SOCKET_HPP
#define REMOTESORT_SOCKET_HPP

#include <optional>
#include <string_view>
#include <netdb.h>

class Socket
{
public:
    enum class State;
private:
    static addrinfo hints;

    int socketFD = -1;
    State state = Socket::State::UNINITIALIZED;
    addrinfo* addresses = nullptr;
public:
    Socket() = default;
    Socket(const char* hostname, const char* port);
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;
    Socket& operator=(Socket &&other);
    Socket(Socket &&other);
    ~Socket();

    int read(void *buffer, size_t nbytes);
    int write(const void *buffer, size_t nbytes);
    int bind();
    int listen(const int waitingRequests);
    Socket* accept();
    int connect();
    int close();
    int setNonBlockingMode();

    State getState() const;

    std::string getHostname() const;
    std::string getIP() const;
    unsigned short getPort() const;
    void printHostname() const;
    void printIP() const;
    void printPort() const;
private:
    Socket(const int socketFD, sockaddr_in address, socklen_t size);
    std::optional<addrinfo*> receiveAddresses(std::string_view hostname, std::string_view port) noexcept;
    std::optional<int> createSocketFD(addrinfo* address) noexcept;

    std::string getHostname(sockaddr *sa, socklen_t size) const;
    unsigned short getPort(sockaddr *s) const;
public:
    enum class State
    {
        FAILED,
        UNINITIALIZED,
        CREATED,
        BOUND,
        LISTENING,
        CONNECTED,
        ACCEPTED,
        CLOSED
    };

    friend std::ostream& operator<< (std::ostream& os, Socket::State state);
};

#endif