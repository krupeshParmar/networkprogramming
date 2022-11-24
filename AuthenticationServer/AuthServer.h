#pragma once
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <vector>
#include "ChatServer.h"
#include "auth.pb.h"

#define DEFAULT_BUFLEN 512
#define TOTAL_ROOMS 5

#pragma comment(lib, "Ws2_32.lib")

class User
{
public:
	
	long ID = 0;
	std::string email = "";
	std::string salt = "";
	std::string hashedPassword = "";
	long userID = 0;
	std::string last_login = "";
	std::string creationDate = "";
};


class AuthServer
{
public:
	AuthServer();
	~AuthServer();
	std::vector<User*> users;

	int Initialize();
	int Accept();
	int ReceiveAndSend();
private:
	int LoadUsers();
	int SaveUsers();
	int CreateUser(std::string email, std::string password, long int);
	int AuthenticateUser(std::string email, std::string password, long int);
	int CreateSocket();
	int BindSocket();
	int Listen();
	int BroadCast();
	int ConnectingSocket();
	int IOCtlSocket();
	ChatServer chatserver;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult, iSelectResult;
	int recvbuflen = DEFAULT_BUFLEN;
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
};

