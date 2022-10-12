#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include "Server.h"
#define WIN32_LEAN_AND_MEAN

int main()
{
	Server server = Server();

	int result = server.Initialize();
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	printf("WSAStartup success: %d\n", result);

	result = server.ReceiveAndSend();

	return result;
}