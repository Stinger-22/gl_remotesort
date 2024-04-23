#include <client.hpp>

#include <iostream>

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		std::cout << "You should pass server hostname and port to communicate with server" << std::endl;
		exit(1);
	}

	Client client(argv[1], argv[2]);
	client.request();

	return 0;
}