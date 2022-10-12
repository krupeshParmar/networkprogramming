#pragma once
#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <vector>
#include "Client.h"
#define DEFAULT_BUFLEN 512
#define TOTAL_ROOMS 5

#pragma comment(lib, "Ws2_32.lib")

class Server {
public:
	Server();
	~Server();
	int Initialize();
	int Accept();
	int ReceiveAndSend();
private:
	int CreateSocket();
	int BindSocket();
	int Listen();
	int Broadcast(std::string , std::string , Client&, int );
	//std::map < std::string, std::vector<Client>> Rooms;
	std::string Rooms[5] = {"general", "resources", "polls", "announcements", "off-topic"};

	char recvbuf[DEFAULT_BUFLEN];
	std::vector<Client> clients;
	Client clientSocket;
	int iResult, iSendResult, iSelectResult;
	int recvbuflen = DEFAULT_BUFLEN;
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
};