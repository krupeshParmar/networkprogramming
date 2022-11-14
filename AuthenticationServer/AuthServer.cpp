#define WIN32_LEAN_AND_MEAN
#include "AuthServer.h"
#define DEFAULT_PORT "5555"
#define SERVER_NAME "GDP_AUTH_SERVER"
#include <iostream>
#include <string>
#include "Buffer.h"
#include "Protocol.h"

AuthServer::AuthServer()
{
}

AuthServer::~AuthServer()
{
	printf("Shutting Down Auth Server\n");
	closesocket(ListenSocket);
	WSACleanup();
}

int AuthServer::Initialize()
{
	printf("\nStarting up GDP Auth Server\n");
	FD_ZERO(&activeSockets);
	//FD_ZERO(&socketReadyForReading);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	iResult = AuthServer::CreateSocket();
	if (iResult != 0)
	{
		printf("Socket creation failed: %d\n", iResult);
		return 1;
	}

	iResult = AuthServer::BindSocket();
	if (iResult != 0)
	{
		printf("Binding failed: %d\n", iResult);
		return 1;
	}

	iResult = AuthServer::Listen();
	if (iResult != 0)
	{
		printf("Listen failed: %d\n", iResult);
		return 1;
	}
	return iResult;
}

// Create listening socket
int AuthServer::CreateSocket()
{
	//printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		return 1;
	}
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		return 1;
	}
	return iResult;
}

// Binding the socket to address
int AuthServer::BindSocket()
{
	//printf("Calling BindSocket . . . ");
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("Bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	return iResult;
}
/// <summary>
/// Listen for connections
/// </summary>
/// <returns></returns>
int AuthServer::Listen()
{
	//printf("Calling Listen . . . ");
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}
