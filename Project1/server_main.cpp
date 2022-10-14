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
*			- Select any any configuration Debug or Release
*			- Right click on "Project1" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "Project1" from Solution Explorer and hit build
*			- The build is now created for Server, open Project1.exe from the selected config directory
*
*	b) How to run this project:
*		- After building, an "exe" file will have been generated inside the "Project1" or "Client" dir/{selected config path}
*
*	c) User inputs:
*		- Keyboard inputs required for Client.exe
*		- Type according to the messages displayed on the screen
*
*/

#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include "Server.h"
#define WIN32_LEAN_AND_MEAN


//	------------------ SERVER ------------------
int main()
{
	Server server = Server();	// Create server instance

	int result = server.Initialize();	// Initialize connection
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	printf("WSAStartup success: %d\n", result);

	result = server.ReceiveAndSend();	// start chat loop

	return result;
}