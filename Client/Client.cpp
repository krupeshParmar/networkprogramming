#include "Client.h"
#include <iostream>
#include <string>
#include <conio.h>

#define DEFAULT_PORT "5555"

std::string getMsg(std::string);

Client::Client()
{

}

Client::~Client()
{
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	closesocket(ConnectSocket);
	WSACleanup();
}

int Client::Initialize(const char* ipadd)
{
	printf("Calling Init . . . ");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	iResult = Client::CreateSocket(ipadd);
	if (iResult != 0)
	{
		return 1;
	}
	
	iResult = Client::ConnectingSocket();
	if (iResult != 0)
	{
		return 1;
	}

	iResult = Client::IOCtlSocket();
	if (iResult != 0)
	{
		return 1;
	}

	return iResult;
}

int Client::CreateSocket(const char* ipadd)
{
	printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	printf("Here 1\n");

	iResult = getaddrinfo(ipadd, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	ptr = result;
	printf("Here 2\n");

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	printf("Here 3\n");
	return 0;
}

int Client::ConnectingSocket()
{
	printf("Calling Connecting Socket . . . ");
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("Failed to connect to server with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		freeaddrinfo(result);
		ConnectSocket = INVALID_SOCKET;
	}

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	printf("Here 4\n");
	return 0;
}

int Client::IOCtlSocket()
{
	DWORD NonBlock = 1;
	iResult = ioctlsocket(ConnectSocket, FIONBIO, &NonBlock);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket to failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	return 0;
}

int Client::SendAndReceive(std::string name)
{
	std::string initMessage = name;
	char recvbuf[DEFAULT_BUFLEN];
	const char* sendbuf = initMessage.c_str();
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes send: %ld\n", iResult);

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	/*iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}*/
	int quit = 0;
	int pause = 0;
	std::string msg = "";
	// Receive data until the server closes the connection

	printf("[Type here]:\t");
	do {
		char ch;
		if (_kbhit())
		{
			ch = _getch();
			if (ch == 27)
				quit = 1;
			if (ch >= 32 && ch <= 126)
			{
				msg += ch;
				std::cout << ch;
			}

			if (ch == 8)
			{
				if (msg.length() > 0)
					msg.pop_back();
				std::cout << "[Type here]:\t" << msg << std::endl;
			}

			if (ch == 13)
			{
				if (msg == "quit")
					quit = 1;
				if (msg[0] == '_')
				{
					if (msg.substr(1, 4) == "join")
					{
						if(msg.length() > 6)
							roomName = msg.substr(6);
					}
				}

				if (msg == "_leave")
				{
					pause = 1;
				}

				if (pause)
					continue;

				iResult = send(ConnectSocket, msg.c_str(), (int)strlen(msg.c_str()), 0);
				if (iResult == SOCKET_ERROR)
				{
					// TODO: REMOVE THIS LATER
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				msg.clear();
				printf("[Type here]:\t");
			}
		}
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		std::string recvmsg = recvbuf;
		if(iResult > 0)
			std::cout << recvmsg << std::endl;
		/*if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else */
		if (iResult == 0)
			printf("Connection closed\n");
		/*else
			printf("recv failed: %d\n", WSAGetLastError());*/
	} while (!quit);
	printf("Exit");

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int Client::Chatting()
{
	return 1;
}
