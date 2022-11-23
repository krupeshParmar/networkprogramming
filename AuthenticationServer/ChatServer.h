#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>
#include "Buffer.h"
class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	ChatServer(SOCKET);
	SOCKET ChatSocket;
	bool connected;
};

