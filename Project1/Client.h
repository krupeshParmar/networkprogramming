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
	std::string roomName;
};