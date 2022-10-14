/*
* 
*	Student name:	Krupesh Ashok Parmar
*	Student number:	1124374
*	Student mail:	k_parmar180076@fanshaweonline.ca
*
*	Program name:	GAME DEVELOPMENT ADVANCED PROGRAMMING
*	Course name:	INFO6016 - Network Programming
*					
*	PROJECT - 1 
*	TCP Client & Server, Chat Program
*	
*	a) How to build this project:
*		-> Client
*			- Select x64 platform for Debug
*			- Or Select any platform for Release
*			- Right click on "Client" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "Client" from Solution Explorer and hit build
*			- The build is now created for Client, open Client.exe from the selected config directory
*		-> Server
*			- Select any platform x86 or x64
*			- Select any configuration Debug or Release
*			- Right click on "Project1" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "Project1" from Solution Explorer and hit build
*			- The build is now created for Server, open Project1.exe from the selected config directory
*
*	b) How to run this project:
*		- After building, an "exe" file will have been generated inside the "Project1" dir/{selected config path}
*
*	c) User inputs:
*		- Keyboard inputs required for Client.exe
*		- Type according to the messages displayed on the screen
*	References: https://learn.microsoft.com/en-us/windows/win32/winsock/using-winsock
*/

#include <stdio.h>
#include "Client.h"
#include <iostream>

//	------------------ CLIENT ------------------

int main(int argc, char* argv[])
{
	// create client instance
	Client client = Client();

	std::string name;

	std::cout << "Enter your name: ";
	std::cin >> name;
	client.clientName = name;		// Store client's name

	int result = client.Initialize("127.0.0.1");	// Initialize connection with pre defined IP address
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	
	result = client.SendAndReceive();				// start chat loop
	if (result != 0)
	{
		printf("Send and receive failed: %d\n", result);
		return 1;
	}
}