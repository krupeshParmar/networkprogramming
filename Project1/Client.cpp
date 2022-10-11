#include "Client.h"

Client::Client() {

}

Client::Client(SOCKET socket)
	:clientSocket(socket)
{
}

Client::~Client() {

}
