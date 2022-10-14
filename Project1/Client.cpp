#include "Client.h"

// Client object for server
Client::Client() {

}

// Client object for server with given socket
Client::Client(SOCKET socket)
	:clientSocket(socket)
{
}

Client::~Client() {

}
