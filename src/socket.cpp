#include <optional>
#include <socket.hpp>

#include <iostream>
#include <cstring>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

//                           IPv4     TCP
addrinfo Socket::hints = {0, AF_INET, SOCK_STREAM};

Socket::Socket(const char* hostname, const char* port)
{
    std::string_view svHostname(hostname, strlen(hostname));
    std::string_view svPort(port, strlen(port));
    std::optional<addrinfo*> addresses = receiveAddresses(svHostname, svPort);
    if (addresses.has_value() == false)
    {
        state = State::FAILED;
        return;
    }
    this->addresses = addresses.value();

    std::optional<int> socket = createSocketFD(addresses.value());
    if (socket.has_value() == false)
    {
        state = State::FAILED;
        return;
    }
    this->socketFD = socket.value();
    state = State::CREATED;
}

Socket& Socket::operator=(Socket &&socket)
{
    this->socketFD = socket.socketFD;
    this->state = socket.state;
    this->addresses = socket.addresses;
    socket.socketFD = -1;
    socket.addresses = nullptr;
    return *this;
}

Socket::Socket(Socket &&other): socketFD(other.socketFD), state(other.state), addresses(other.addresses)
{
    this->socketFD = -1;
    this->addresses = nullptr;
}

Socket::~Socket()
{
    if (addresses != nullptr)
    {
        freeaddrinfo(addresses);
    }
    if (state != State::CLOSED)
    {
        close();
    }
}

Socket::State Socket::getState() const
{
    return state;
}

std::optional<addrinfo*> Socket::receiveAddresses(std::string_view hostname, std::string_view port) noexcept
{
    std::clog << "[Debug] Resolving hostname.\n";
    addrinfo *addresses;
    if (getaddrinfo(hostname.data(), port.data(), &hints, &addresses) != 0)
    {
        std::clog << "[Error] Failed to resolve addresses: getaddrinfo(): " << std::system_category().message(errno) << std::endl;
        return std::nullopt;
    }
    if (addresses->ai_next)
    {
        std::clog << "[Debug] Found multiple addresses. This app selects automatically the first address.\n";
    }
    return addresses;
}

std::optional<int> Socket::createSocketFD(addrinfo* address) noexcept
{
    if (address == nullptr)
    {
        std::clog << "[Error] createSocket(): address is nullptr." << std::endl;
        return std::nullopt;
    }
    std::clog << "[Debug] Creating socket.\n";
    int newSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (newSocket == -1)
    {
        std::clog << "[Error] Failed to create socket: socket(): " << std::system_category().message(errno) << std::endl;
        return -1;
    }
    return newSocket;
}

int Socket::bind()
{
    std::clog << "[Debug] [Socket " << socketFD << "] Binding to the resolved address." << std::endl;
    int bindResult = ::bind(socketFD, addresses->ai_addr, addresses->ai_addrlen);
    if (bindResult == -1)
    {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to bind socket to address " << getHostname() << ":" << getPort() << " : bind(): " << std::system_category().message((errno)) << std::endl;
    }
    else
    {
        state = State::BOUND;
    }
    return bindResult;
}

int Socket::listen(const int waitingRequests)
{
    std::clog << "[Debug] [Socket " << socketFD << "] Activating listening mode.\n";
    int listenResult = ::listen(socketFD, waitingRequests);
    if (listenResult == -1) {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to activate listening mode: listen(): " << std::system_category().message((errno)) << std::endl;
    }
    else
    {
        state = State::LISTENING;
        std::clog << "[Info] [Socket " << socketFD << "] Listening for incoming connections at " << getHostname() << ":" << getPort() << std::endl;
    }
    return listenResult;
}

Socket* Socket::accept()
{
    struct sockaddr_in address;
    socklen_t size = sizeof(address);
    int socket = ::accept(socketFD, (struct sockaddr *) &address, &size);
    if (socket == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // std::cout << "No available sockets to accept\n";
        }
        else
        {
            std::clog << "[Error] [Socket " << socketFD << "] Failed to accept socket: accept(): " << std::system_category().message(errno) << std::endl;
        }
        return nullptr;
    }
    return new Socket(socket, address, size);
}

int Socket::connect()
{
    std::clog << "[Info] Connecting to (" << getHostname() << ":" << getPort() << ")\n";
    if (::connect(socketFD, addresses->ai_addr, addresses->ai_addrlen) == -1)
    {
        std::clog << "[Error] Connection to server failed: connect(): " << std::system_category().message(errno) << std::endl;
        return -1;
    }
    std::clog << "[Info] Successfully connected to " << getHostname() << ":" << getPort() << std::endl;
    state = Socket::State::CONNECTED;
    return 0;
}

int Socket::close()
{
    int closeResult = -1;
    if (socketFD != -1)
    {
        int backupSocketFD = socketFD;
        closeResult = ::close(socketFD);
        if (closeResult == -1)
        {
            std::clog << "[Error] [Socket " << backupSocketFD << "] Failed to close socket: close(): " << std::system_category().message(errno) << std::endl;
        }
        else
        {
            state = State::CLOSED;
        }
    }
    return closeResult;
}

int Socket::read(void *buffer, size_t nbytes)
{
    if ((state != State::ACCEPTED) && (state != State::CONNECTED))
    {
        std::clog << "[Error] [Socket " << socketFD << "] Can't read in the " << state << " state." << std::endl;
        return -1;
    }
    return ::read(socketFD, buffer, nbytes);
}

int Socket::write(const void *buffer, size_t nbytes)
{
    if ((state != State::ACCEPTED) && (state != State::CONNECTED))
    {
        std::clog << "[Error] [Socket " << socketFD << "] Can't write in the " << state << " state." << std::endl;
        return -1;
    }
    return ::write(socketFD, buffer, nbytes);
}

int Socket::setNonBlockingMode()
{
    int status = fcntl(socketFD, F_SETFL, fcntl(socketFD, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1)
    {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to set socket to non-blocking mode: fcntl(): " << std::system_category().message(errno) << std::endl;
    }
    return status;
}

Socket::Socket(const int socketFD, sockaddr_in address, socklen_t size)
{
    if (socketFD == -1)
    {
        this->state = State::FAILED;
    }
    else
    {
        this->socketFD = socketFD;
        // TODO haven't make up with better solution to initialize addresses field.
        std::string_view svHostname, svPort;
        svHostname = getHostname((sockaddr *) &address, size);
        svPort = std::to_string(getPort((sockaddr *) &address));
        std::optional<addrinfo*> addresses = receiveAddresses(svHostname, svPort);
        if (addresses.has_value() == false)
        {
            std::clog << "[Warning] [Socket " << socketFD << "] Can't get addrinfo from accepted socket.\n";
        }
        else
        {
            this->addresses = addresses.value();
        }
        this->state = State::ACCEPTED;
    }
}

std::string Socket::getHostname() const
{
    char addressHostname[NI_MAXHOST];
    if (getnameinfo(addresses->ai_addr, addresses->ai_addrlen, addressHostname, sizeof(addressHostname), nullptr, 0, NI_NAMEREQD))
    {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to get hostname: getnameinfo(): " << std::system_category().message(errno) << std::endl;
    }
    return addressHostname;
}

std::string Socket::getIP() const
{
    char addressIP[NI_MAXHOST];
    if (getnameinfo(addresses->ai_addr, addresses->ai_addrlen, addressIP, sizeof(addressIP), nullptr, 0, NI_NUMERICSERV | NI_NUMERICHOST))
    {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to get ip: getnameinfo(): " << std::system_category().message(errno) << std::endl;
    }
    return addressIP;
}

unsigned short Socket::getPort() const
{
    struct sockaddr *s = addresses->ai_addr;
    return ntohs(((sockaddr_in*)s)->sin_port);
}

void Socket::printHostname() const
{
    std::clog << "[Info] [Socket " << socketFD << "] Hostname: " << getHostname() << std::endl;
}

void Socket::printIP() const
{
    std::clog << "[Info] [Socket " << socketFD << "] IP: " << getIP() << std::endl;
}

void Socket::printPort() const
{
    std::clog << "[Info] [Socket " << socketFD << "] Port: " << getPort() << std::endl;
}

std::string Socket::getHostname(sockaddr *s, socklen_t size) const
{
    char addressHostname[NI_MAXHOST];
    if (getnameinfo(s, size, addressHostname, sizeof(addressHostname), nullptr, 0, NI_NAMEREQD))
    {
        std::clog << "[Error] [Socket " << socketFD << "] Failed to get hostname: getnameinfo(): " << std::system_category().message(errno) << std::endl;
    }
    return addressHostname;
}

unsigned short Socket::getPort(sockaddr *s) const
{
    return ntohs(((sockaddr_in*)s)->sin_port);
}

std::ostream& operator<< (std::ostream& os, Socket::State state)
{
    switch (state)
    {
        case Socket::State::FAILED: return os << "FAILED";
        case Socket::State::UNINITIALIZED: return os << "UNINITIALIZED";
        case Socket::State::CREATED: return os << "CREATED";
        case Socket::State::BOUND: return os << "BOUND";
        case Socket::State::LISTENING: return os << "LISTENING";
        case Socket::State::CONNECTED: return os << "CONNECTED";
        case Socket::State::ACCEPTED: return os << "ACCEPTED";
        case Socket::State::CLOSED: return os << "CLOSED";
    };
    return os << static_cast<int>(state);
}
