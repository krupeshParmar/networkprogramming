#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <string>
#define DEFAULT_BUFLEN 512
#pragma comment(lib, "Ws2_32.lib")

class Client {
public:
	Client();
	~Client();
	int Initialize(const char*);
	int SendAndReceive(std::string);
	int Chatting();
private:
	int CreateSocket(const char*);
	int ConnectingSocket();
	int IOCtlSocket();

	int iResult, recvbuflen = DEFAULT_BUFLEN;;
	WSADATA wsaData;
	std::string roomName;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
};