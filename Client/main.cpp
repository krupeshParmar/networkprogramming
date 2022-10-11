#include <stdio.h>
#include "Client.h"
#include <iostream>

int main(int argc, char* argv[])
{
	Client client = Client();

	std::string name;

	std::cout << "Enter your name: ";
	std::cin >> name;

	int result = client.Initialize("127.0.0.1");
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	
	result = client.SendAndReceive(name);
	if (result != 0)
	{
		printf("Send and receive failed: %d\n", result);
		return 1;
	}
	printf("WSAStartup success: %d\n", result);
}