#include <client.hpp>

#include <iostream>

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		std::cout << "You should pass server hostname and port to communicate with server\n";
        std::cout << "General command: client [server-hostname] [port]\n";
        std::cout << "Example of command: client stinger 33333" << std::endl;
		return 1;
	}

	Client client(argv[1], argv[2]);
	client.request();

	return 0;
}