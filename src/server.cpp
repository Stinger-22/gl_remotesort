#include <cstdlib>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <util.hpp>


void echoServe(int acceptedSocketFD);

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		std::cout << "You should pass port on which server will listen" << std::endl;
		exit(1);
	}
	int port = parsePort(argv[1]);
	std::cout << "Starting echo-server on port " << port << "!" << std::endl;

	// FD - File Descriptor
	int socketFD, acceptedSocketFD;
	socklen_t clientSize;
	struct sockaddr_in serverAddress, clientAddress;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
        reportError("ERROR opening socket", 1);
	}

	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Set hostname
	serverAddress.sin_port = htons(port);

	if (bind(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1)
	{
		reportError("ERROR on binding", 1);
	}

	listen(socketFD, 5);

	clientSize = sizeof(clientAddress);

	int pid;
	while (1)
	{
		acceptedSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &clientSize);
		if (acceptedSocketFD == -1)
		{
			reportError("ERROR on accept", 1);
		}
		pid = fork();
		if (pid == -1)
		{
             reportError("ERROR on fork", 1);
		}
		else if (pid == 0)
		{
			close(socketFD);
			echoServe(acceptedSocketFD);
			exit(0);
		}
		close(acceptedSocketFD);
	}
	close(socketFD);
	return 0;
}

void echoServe(int acceptedSocketFD)
{
	int messageSize;
	char buffer[256];

	std::memset(buffer, 0, sizeof(buffer));
	messageSize = read(acceptedSocketFD, buffer, 255);
	if (messageSize < 0)
	{
		reportError("ERROR reading client message from socket", 1);
	}
	std::cout << "Received message: " << buffer << std::endl;


	messageSize = write(acceptedSocketFD, buffer, messageSize);
	if (messageSize < 0)
	{
		reportError("ERROR writing echo to socket", 1);
	}
}