#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
	std::cout << "Starting server!" << std::endl;

	// FD - File Descriptor
	int socketFD, port;
	struct sockaddr_in serv_addr, cli_addr;
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	return 0;
}