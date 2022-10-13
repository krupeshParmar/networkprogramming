#define WIN32_LEAN_AND_MEAN
#include "Server.h"
#define DEFAULT_PORT "5555"
#include <iostream>
#include <string>
#include <sstream>
#include "Buffer.h"
#include "Protocol.h"

Server::Server()
{

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
		uint8_t buf[buflen];

		Client client;
		client.clientSocket = clientSocket;
		client.connected = true;

		int recvResult = recv(clientSocket, (char*)&buf, buflen, 0);
		if (recvResult > 0)
		{
			std::cout << recvResult << std::endl;
			Buffer buffer = Buffer(recvResult);
			for (int i = 0; i < recvResult; i++)
			{
				buffer.m_Buffer[i] = buf[i];
			}
			int packetlength = buffer.ReadInt32LE();
			int messageType = buffer.ReadInt16LE();
			int senderNameSize = buffer.ReadInt32LE();
			std::string senderName = buffer.ReadString(senderNameSize);
			int roomNameSize = buffer.ReadInt32LE();
			std::string roomName = buffer.ReadString(roomNameSize);
			int msgSize = buffer.ReadInt32LE();
			std::string msg = buffer.ReadString(msgSize);
			client.clientName = msg;
			client.roomName = "";
			if (msg == client.clientName)
			{
				MessagePacket packet;

				packet.header.messageType = WELCOME;
				packet.content.roomName = "";
				packet.content.senderName = "SERVER";
				packet.content.message = "Welcome " + client.clientName + "\nRooms you can join:\n";
				for (int i = 0; i < TOTAL_ROOMS; i++)
				{
					packet.content.message += i + 1 + ")" + Rooms[i] + "\n";
				}
				packet.content.message += "\nUse _join room_name command to join a room\n";
				packet.content.message += "\nUse _leave command to leave the room\n";
				packet.content.message += "\nUse _help command to get help\n";

				packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
					+ 4 + packet.content.roomName.size()
					+ 4 + packet.content.message.size();
				Buffer buffer = Buffer(packet.header.packetLength);
				buffer.WriteInt32LE(packet.header.packetLength);
				buffer.WriteInt16LE(packet.header.messageType);
				buffer.WriteInt32LE(packet.content.senderName.size());
				buffer.WriteString(packet.content.senderName);
				buffer.WriteInt32LE(packet.content.roomName.size());
				buffer.WriteString(packet.content.roomName);
				buffer.WriteInt32LE(packet.content.message.size());
				buffer.WriteString(packet.content.message);
				iSendResult = send(client.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
			}
		}

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

int Server::Broadcast(std::string msg, std::string roomName, Client& sender, int messageType)
{
	if (messageType == HELP)
	{
		MessagePacket packet;
		packet.content.message += "\Rooms you can join:\n";
		for (int i = 0; i < TOTAL_ROOMS; i++)
		{
			packet.content.message += i + 1 + ")" + Rooms[i] + "\n";
		}
		packet.content.message += "\nUse _join room_name command to join a room\n";
		packet.content.message += "\nUse _leave command to leave the room\n";
		packet.content.message += "\nUse _help command to get help\n";
		packet.header.messageType = HELP;
		packet.content.roomName = roomName;
		packet.content.senderName = "SERVER";
		packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
			+ 4 + packet.content.roomName.size()
			+ 4 + packet.content.message.size();

		Buffer buffer = Buffer(packet.header.packetLength);
		buffer.WriteInt32LE(packet.header.packetLength);
		buffer.WriteInt16LE(packet.header.messageType);
		buffer.WriteInt32LE(packet.content.senderName.size());
		buffer.WriteString(packet.content.senderName);
		buffer.WriteInt32LE(packet.content.roomName.size());
		buffer.WriteString(packet.content.roomName);
		buffer.WriteInt32LE(packet.content.message.size());
		buffer.WriteString(packet.content.message);
		iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
		return iSendResult;
	}
	bool roomFound = false;

	for (int i = 0; i < TOTAL_ROOMS; i++)
		if (roomName == Rooms[i])
			roomFound = true;
	if (roomFound)
	{
		std::cout << "[ "  << roomName << "] " << "[ " << sender.clientName << "] " << msg << std::endl;
		
		if (messageType == JOIN)
		{
			MessagePacket packet;

			packet.header.messageType = JOIN;
			packet.content.roomName = roomName;
			packet.content.senderName = sender.clientName;
			packet.content.message = "\nWelcome to "+roomName + "\n";
			packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
				+ 4 + packet.content.roomName.size()
				+ 4 + packet.content.message.size();

			Buffer buffer = Buffer(packet.header.packetLength);
			buffer.WriteInt32LE(packet.header.packetLength);
			buffer.WriteInt16LE(packet.header.messageType);
			buffer.WriteInt32LE(packet.content.senderName.size());
			buffer.WriteString(packet.content.senderName);
			buffer.WriteInt32LE(packet.content.roomName.size());
			buffer.WriteString(packet.content.roomName);
			buffer.WriteInt32LE(packet.content.message.size());
			buffer.WriteString(packet.content.message);
			sender.roomName = roomName;
			iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
		}

		for (int i = 0; i < clients.size(); i++)
		{
			MessagePacket packet;

			packet.header.messageType = MESSAGE;
			packet.content.roomName = roomName;
			packet.content.senderName = sender.clientName;
			packet.content.message = msg;
			packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
				+ 4 + packet.content.roomName.size()
				+ 4 + packet.content.message.size();

			Buffer buffer = Buffer(packet.header.packetLength);
			buffer.WriteInt32LE(packet.header.packetLength);
			buffer.WriteInt16LE(packet.header.messageType);
			buffer.WriteInt32LE(packet.content.senderName.size());
			buffer.WriteString(packet.content.senderName);
			buffer.WriteInt32LE(packet.content.roomName.size());
			buffer.WriteString(packet.content.roomName);
			buffer.WriteInt32LE(packet.content.message.size());
			buffer.WriteString(packet.content.message);

			if (clients[i].roomName == roomName)
			{
				iSendResult = send(clients[i].clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
			}
		}

		return iSendResult;
	}
	else {
		if (messageType == JOIN)
		{
			MessagePacket packet;
			packet.header.messageType = 1;
			packet.content.roomName = roomName;
			packet.content.senderName = "SERVER";
			packet.content.message = "No group named " + roomName + " exists.";
			packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
				+ 4 + packet.content.roomName.size()
				+ 4 + packet.content.message.size();

			Buffer buffer = Buffer(packet.header.packetLength);
			buffer.WriteInt32LE(packet.header.packetLength);
			buffer.WriteInt16LE(packet.header.messageType);
			buffer.WriteInt32LE(packet.content.senderName.size());
			buffer.WriteString(packet.content.senderName);
			buffer.WriteInt32LE(packet.content.roomName.size());
			buffer.WriteString(packet.content.roomName);
			buffer.WriteInt32LE(packet.content.message.size());
			buffer.WriteString(packet.content.message);

			iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), 512, 0);
		}
		else
		{
			MessagePacket packet;
			packet.header.messageType = 1;
			packet.content.roomName = roomName;
			packet.content.senderName = "SERVER";
			packet.content.message = "Please join a group first\nGroups you can join:\n";
			for (int i = 0; i < TOTAL_ROOMS; i++)
			{
				packet.content.message += i + 1 + ")" + Rooms[i] + "\n";
			}
			packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
				+ 4 + packet.content.roomName.size()
				+ 4 + packet.content.message.size();

			Buffer buffer = Buffer(packet.header.packetLength);
			buffer.WriteInt32LE(packet.header.packetLength);
			buffer.WriteInt16LE(packet.header.messageType);
			buffer.WriteInt32LE(packet.content.senderName.size());
			buffer.WriteString(packet.content.senderName);
			buffer.WriteInt32LE(packet.content.roomName.size());
			buffer.WriteString(packet.content.roomName);
			buffer.WriteInt32LE(packet.content.message.size());
			buffer.WriteString(packet.content.message);

			iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), 512, 0);
			return iSendResult;
		}
	}

	if(messageType == LEAVE)
		sender.roomName = "";

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
				uint8_t data[buflen];

				int recvResult = recv(client.clientSocket, (char*)&data[0], buflen, 0);

				if (recvResult == 0 && client.connected)
				{
					printf("Client disconnected\n");
					client.connected = false;
					continue;
				}

				if (recvResult > 0)
				{
					int sendResult = -10;
					Buffer buffer = Buffer(recvResult);
					for (int i = 0; i < recvResult; i++)
					{
						buffer.m_Buffer[i] = data[i];
					}
					int packetlength = buffer.ReadInt32LE();
					int messageType = buffer.ReadInt16LE();
					int senderNameSize = buffer.ReadInt32LE();
					std::string senderName = buffer.ReadString(senderNameSize);
					int roomNameSize = buffer.ReadInt32LE();
					std::string roomName = buffer.ReadString(roomNameSize);
					int msgSize = buffer.ReadInt32LE();
					std::string msg = buffer.ReadString(msgSize);
					std::cout << msg << std::endl;
					switch (messageType)
					{
					case JOIN:
						sendResult = Server::Broadcast(senderName + " has joined " + roomName + "\n", roomName, client, messageType);
						break;
					case LEAVE:
						sendResult = Server::Broadcast(senderName + " has left " + roomName + "\n", roomName, client, messageType);
						break;
					case MESSAGE:
						sendResult = Server::Broadcast(msg + "\n", roomName, client, messageType);
						break;
					case HELP:
						sendResult = Server::Broadcast(msg + "\n", roomName, client, messageType);
						break;
					default:
						break;
					}

					if (sendResult < 0)
						printf("\nError\n");
				}
				

				/*std::string msg = buf;
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
				iSendResult = send(client.clientSocket, buf, recvResult, 0);*/
			}
		}
	}
}
