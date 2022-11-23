#include "Authenticator.h"

Authenticator::Authenticator()
{
}

Authenticator::~Authenticator()
{
}

Authenticator::Authenticator(SOCKET socket)
	: AuthSocket(socket)
{
}
