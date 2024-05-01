#ifndef REMOTESORT_CLIENT_HPP
#define REMOTESORT_CLIENT_HPP

#include <netdb.h>
#include <string>

class Client
{
private:
	std::string serverHostname;
	std::string port;

	int clientSocket;

public:
	Client(const char* serverHostname, const char* port);
	~Client() = default;


	void request();
private:
	addrinfo* resolveServerAddress() noexcept;
	int createClientSocket(addrinfo* serverAddress) noexcept;
	int connectToServer(addrinfo* serverAddress) noexcept;
	void askSorting();
	void receiveAnswer();
};

#endif