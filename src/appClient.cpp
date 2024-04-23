#include <cstdlib>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <util.hpp>

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		std::cout << "You should pass server hostname and port to communicate with server" << std::endl;
		exit(1);
	}
	int port = parsePort(argv[2]);
	const char* hostname = argv[1];
	std::cout << "Сlient to echo-server is started on port " << port << " connecting to server: " << hostname << std::endl;

	int socketFD, messageSize;
	char buffer[256];
	struct sockaddr_in serverAddress;
	struct hostent *server;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
        reportError("ERROR opening socket", 1);
	}

	// TODO man says gethostbyname is deprecated
	server = gethostbyname(hostname);
	if (server == NULL)
	{
		reportError("ERROR, no such host", 1);
	}

    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    std::memcpy( (char *) &serverAddress.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) != 0)
    {
    	reportError("ERROR connecting", 1);
    }

    std::cout << "Please enter the message: ";
    std::memset(buffer, 0, sizeof(buffer));
    std::cin.getline(buffer, 255);
    messageSize = write(socketFD, buffer, strlen(buffer));
    if (messageSize < 0)
    {
    	reportError("ERROR writing to socket", 1);
    }

    std::memset(buffer, 0, sizeof(buffer));
    messageSize = read(socketFD, buffer, 255);
    if (messageSize < 0)
    {
    	reportError("ERROR reading from socket", 1);
    }
    std::cout << "Echo: " << buffer << std::endl;
    close(socketFD);

	return 0;
}