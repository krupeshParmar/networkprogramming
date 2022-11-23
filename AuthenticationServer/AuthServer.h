#pragma once
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <vector>
#include "ChatServer.h"
#include "auth.pb.h"
#define DEFAULT_BUFLEN 512
#define TOTAL_ROOMS 5

#pragma comment(lib, "Ws2_32.lib")


class AuthServer
{
public:
	AuthServer();
	~AuthServer();

	int Initialize();
	int Accept();
	int ReceiveAndSend();
private:
	int CreateSocket();
	int BindSocket();
	int Listen();
	int BroadCast();
	int ConnectingSocket();
	int IOCtlSocket();
	ChatServer chatserver;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult, iSelectResult;
	int recvbuflen = DEFAULT_BUFLEN;
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
};

