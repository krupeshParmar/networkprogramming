#include <iostream>
#include <WinSock2.h>
#include "AuthServer.h"
#define WIN32_LEAN_AND_MEAN

int main()
{
	AuthServer authServer =
		AuthServer();	// Create server instance

	int result = authServer.Initialize();	// Initialize connection
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	printf("WSAStartup success: %d\n", result);
	result = authServer.ReceiveAndSend();

	return 0;
}