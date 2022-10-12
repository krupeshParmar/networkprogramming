#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>
#include "Buffer.h"

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