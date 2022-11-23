#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>
#include "Buffer.h"

class Authenticator
{
public:
	Authenticator();
	~Authenticator();
	Authenticator(SOCKET socket);
	SOCKET AuthSocket = INVALID_SOCKET;
	bool connected;

};

