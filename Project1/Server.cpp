#define WIN32_LEAN_AND_MEAN
#include "Server.h"
#define DEFAULT_PORT "5555"
#include <iostream>
#include <string>
#include <sstream>

Server::Server()
{
	std::vector<Client> emptyClients1 =  std::vector<Client>();
	std::vector<Client> emptyClients2 =  std::vector<Client>();
	std::vector<Client> emptyClients3 =  std::vector<Client>();
	std::vector<Client> emptyClients4 =  std::vector<Client>();
	std::vector<Client> emptyClients5 =  std::vector<Client>();
	Rooms.insert({"general", emptyClients1 });
	Rooms.insert({"resources", emptyClients2 });
	Rooms.insert({"polls", emptyClients3 });
	Rooms.insert({"announcements", emptyClients4 });
	Rooms.insert({"off-topic", emptyClients5 });

}

Server::~Server()
{
	printf("Shutting down\n");
	closesocket(ListenSocket);
	WSACleanup();
}

int Server::Initialize()
{
	printf("Calling Init . . . ");

	FD_ZERO(&activeSockets);
	FD_ZERO(&socketsReadyForReading);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); 
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	iResult = Server::CreateSocket();
	if (iResult != 0)
	{
		printf("Socket creation failed: %d\n", iResult);
		return 1;
	}

	iResult = Server::BindSocket();
	if (iResult != 0)
	{
		printf("Binding failed: %d\n", iResult);
		return 1;
	}

	iResult = Server::Listen();
	if (iResult != 0)
	{
		printf("Listen failed: %d\n", iResult);
		return 1;
	}

	return iResult;
}

int Server::CreateSocket()
{
	printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		return 1;
	}
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		return 1;
	}
	return iResult;
}

int Server::BindSocket()
{
	printf("Calling BindSocket . . . ");
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("Bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	return iResult;
}

int Server::Listen()
{
	printf("Calling Listen . . . ");
	iResult = listen(ListenSocket, SOMAXCONN);
	if ( iResult == SOCKET_ERROR )
	{
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int Server::Accept()
{
	printf("Calling Accept . . . ");
	SOCKET clientSocket = accept(ListenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Accept failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		const int buflen = 128;
		char buf[buflen];

		Client client;
		client.clientSocket = clientSocket;
		client.connected = true;

		int recvResult = recv(clientSocket, buf, buflen, 0);
		std::string msg = buf;
		client.clientName = msg;
		client.roomName = "";

		clients.push_back(client);
	}
	return 0;
}

//int Server::ReceiveAndSend()
//{
//	int byteSentCounts = 0, byteRecvCounts = 0;
//	int quit = 0;
//	do
//	{
//		iResult = recv(clientSocket.clientSocket, recvbuf, recvbuflen, 0);
//		if (recvbuf == "quit")
//			quit = 1;
//		if (iResult > 0)
//		{
//			std::string msg = "";
//			msg = "\n" + std::string(recvbuf) + "\n";
//			byteRecvCounts++;
//			std::cout << msg;
//			// Echo the buffer back to the sender
//			iSendResult = send(clientSocket.clientSocket, recvbuf, iResult, 0);
//			if (iSendResult == SOCKET_ERROR)
//			{
//				printf("Send failed: %d\n", WSAGetLastError());
//				closesocket(clientSocket.clientSocket);
//				WSACleanup();
//				return 1;
//			}
//			//printf("Bytes sent: %d\n", iSendResult);
//		}
//		/*else if (iResult == 0)
//			printf("Connection closing . . .");*/
//		else {
//			printf("recv failed: %d\n", WSAGetLastError());
//			closesocket(clientSocket.clientSocket);
//			WSACleanup();
//			return 1;
//		}
//
//	} while (quit != 1);
//	return iResult;
//}

int Server::Broadcast(std::string msg, std::string roomName)
{
	std::map < std::string, std::vector<Client>>::iterator it;

	it = Rooms.find(roomName); 
	
	if (it != Rooms.end())
	{
		std::vector<Client>* clientsInTheRoom = &Rooms[roomName];
		for (int i = 0; i < clientsInTheRoom->size(); i++)
		{
			Client client = clientsInTheRoom->at(i);
			iSendResult = send(client.clientSocket, msg.c_str(), 512, 0);
			printf("\nClient: %s\n", client.clientName);
		}
		//iSendResult = send(client.clientSocket, buf, recvResult, 0);
	}

	return iSendResult;
	
}

int Server::ReceiveAndSend()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000; // 500 milliseconds, half a second

	int selectResult;

	int num = 0;
	while (1)
	{
		FD_ZERO(&socketsReadyForReading);

		FD_SET(ListenSocket, &socketsReadyForReading);

		for (int i = 0; i < clients.size(); i++)
		{
			Client& client = clients[i];
			if (client.connected)
			{
				FD_SET(client.clientSocket, &socketsReadyForReading);
			}
		}

		iSelectResult = select(0, &socketsReadyForReading, NULL, NULL, &tv);
		if (iSelectResult == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			return 1;
		}
		printf(".");

		if (FD_ISSET(ListenSocket, &socketsReadyForReading))
		{
			printf("\n");
			Server::Accept();
		}

		for (int i = clients.size() - 1; i >= 0; i--)
		{
			Client& client = clients[i];
			if (client.connected == false)
				continue;

			if (FD_ISSET(client.clientSocket, &socketsReadyForReading))
			{
				const int buflen = 128;
				char buf[buflen];

				int recvResult = recv(client.clientSocket, buf, buflen, 0);

				if (recvResult == 0)
				{
					printf("Client disconnected\n");
					client.connected = false;
					continue;
				}
				std::string msg = buf;
				msg = msg.substr(0, recvResult);
				if (msg[0] == '_')
				{
					printf("Here1\n");
					if (msg.substr(1, 4) == "join")
					{
						printf("Here2\n");
						std::string roomName = msg.substr(6);

						std::map < std::string, std::vector<Client>>::iterator it;

						it = Rooms.find(roomName);
						if (it != Rooms.end())
						{
							printf("Here3\n");
							std::vector<Client>* clientsInTheRoom = &Rooms[roomName];
							clientsInTheRoom->push_back(client);
							Server::Broadcast(client.clientName + " has joined " + roomName, roomName);
							printf("%s\n\n", client.roomName.c_str());
							continue;
						}
					}
				}

				msg.append(buf);
				std::cout << msg << std::endl;
				iSendResult = send(client.clientSocket, buf, recvResult, 0);
			}
		}
	}
}
