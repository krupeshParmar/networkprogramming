#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>
#include "Buffer.h"

// Client class for server
class Client {
public:
	Client();
	~Client();
	Client(SOCKET socket);

	SOCKET clientSocket;
	std::string email;
	int requestID;
	bool connected;
	bool authenticated = false;
	std::string clientName;
	int rooms[5] = {0,0,0,0,0};
	int roomsJoined = 0;
};