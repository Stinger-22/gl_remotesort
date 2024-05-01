#include <server.hpp>

#include <iostream>
#include <cstdio>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cout << "You should pass port on which server will listen\n";
        std::cout << "General command: server [port]\n";
        std::cout << "Example of the command: server 33333" << std::endl;
        return 1;
    }

    Server server(argv[1]);
    if (server.start() == -1)
    {
        std::cout << "Server failed to start" << std::endl;
    }

    char command;
    while (command != 'q')
    {
        command = getchar();
    }
    return 0;
}