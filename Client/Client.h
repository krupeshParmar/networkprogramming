#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <string>
#include "auth.pb.h"
#define DEFAULT_BUFLEN 512
#pragma comment(lib, "Ws2_32.lib")

// Client class
class Client {
public:
	Client();
	~Client();
	int Initialize(const char*);
	int SendAndReceive();
	std::string clientName = "";
private:
	bool authenticated = false;
	int CreateSocket(const char*);
	int ConnectingSocket();
	int IOCtlSocket();
	int GetRoomId(std::string);
	std::string Rooms[5] = { "general", "resources", "polls", "announcements", "off-topic" };

	int iResult, recvbuflen = DEFAULT_BUFLEN;;
	WSADATA wsaData;
	int roomsJoined[5] = {0,0,0,0,0};
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
};