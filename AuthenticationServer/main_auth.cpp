/*
*
*	Student name:	Krupesh Ashok Parmar
*	Student number:	1124374
*	Student mail:	k_parmar180076@fanshaweonline.ca
*
*	Program name:	GAME DEVELOPMENT ADVANCED PROGRAMMING
*	Course name:	INFO6016 - Network Programming
*
*	PROJECT - 2
*	Auth Server
*
*	a) How to build this project:
*		-> Client
*			- Select x64 platform for Debug
*			- Or Select any platform for Release
*			- Right click on "Client" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "Client" from Solution Explorer and hit build
*			- The build is now created for Client, open Client.exe from the selected config directory
*		-> Server
*			- Select any platform x64
*			- Select any configuration Debug or Release
*			- Right click on "Project1" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "Project1" from Solution Explorer and hit build
*			- The build is now created for Server, open Project1.exe from the selected config directory
*		-> Server
*			- Select any platform x64
*			- Select any configuration Debug or Release
*			- Right click on "AuthenticationServer" from Solution Explorer and "Clean" the solution (during first time build only)
*			- Right click on "AuthenticationServer" from Solution Explorer and hit build
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