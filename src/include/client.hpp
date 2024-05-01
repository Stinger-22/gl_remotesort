#ifndef REMOTESORT_CLIENT_HPP
#define REMOTESORT_CLIENT_HPP

#include <socket.hpp>
#include <string>

class Client
{
private:
	std::string serverHostname;
	std::string port;

	Socket clientSocket;
public:
	Client(const char* serverHostname, const char* port);
	~Client() = default;

	void request();
private:
	void askSorting();
	void receiveAnswer();
};

#endif