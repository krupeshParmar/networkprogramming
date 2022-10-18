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
	bool connected;
	std::string clientName;
	int rooms[5] = {0,0,0,0,0};
	int roomsJoined = 0;
};