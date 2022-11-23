#include "ChatServer.h"

ChatServer::ChatServer()
{
}

ChatServer::~ChatServer()
{
}

ChatServer::ChatServer(SOCKET socket)
	:ChatSocket(socket)
{
}
