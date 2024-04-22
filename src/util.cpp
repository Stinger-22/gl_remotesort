#include <climits>
#include <util.hpp>

#include <cstdlib>
#include <cstdio>

void reportError(const char* msg, int terminate)
{
	perror(msg);
	if (terminate)
	{
		exit(-1);
	}
}

int parsePort(const char* portStr)
{
	long parsedPort = strtol(portStr, NULL, 10);
	if ( (parsedPort == 0L) || (parsedPort == LONG_MAX) || (parsedPort == LONG_MIN) )
	{
		reportError("ERROR invalid port number", 1);
	}
	if (parsedPort > 65535)
	{
		reportError("ERROR invalid port number", 1);
	}
	return (int) parsedPort;
}