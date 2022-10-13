#include "Client.h"
#include <iostream>
#include <string>
#include <conio.h>
#include "../Project1/Buffer.h"
#include "../Project1/Protocol.h"

#define DEFAULT_PORT "5555"

std::string getMsg(std::string);

Client::Client()
{
	roomName = "";
}

Client::~Client()
{
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	closesocket(ConnectSocket);
	WSACleanup();
}

int Client::Initialize(const char* ipadd)
{
	printf("Calling Init . . . ");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	iResult = Client::CreateSocket(ipadd);
	if (iResult != 0)
	{
		return 1;
	}
	
	iResult = Client::ConnectingSocket();
	if (iResult != 0)
	{
		return 1;
	}

	iResult = Client::IOCtlSocket();
	if (iResult != 0)
	{
		return 1;
	}

	return iResult;
}

int Client::CreateSocket(const char* ipadd)
{
	printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	printf("Here 1\n");

	iResult = getaddrinfo(ipadd, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	ptr = result;
	printf("Here 2\n");

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	printf("Here 3\n");
	return 0;
}

int Client::ConnectingSocket()
{
	printf("Calling Connecting Socket . . . ");
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("Failed to connect to server with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		freeaddrinfo(result);
		ConnectSocket = INVALID_SOCKET;
	}

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	printf("Here 4\n");
	return 0;
}

int Client::IOCtlSocket()
{
	DWORD NonBlock = 1;
	iResult = ioctlsocket(ConnectSocket, FIONBIO, &NonBlock);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket to failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	return 0;
}

int Client::SendAndReceive()
{
	MessagePacket packet;
	packet.header.messageType = MESSAGE;
	packet.content.senderName = clientName;
	std::cout << "Client name" << clientName;
	packet.content.roomName = "";
	packet.content.message = clientName;
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
	char* bufPtr = (char*)&(buffer.m_Buffer[0]);
	iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);

	std::cout << iResult << std::endl;
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	int quit = 0;
	int pause = 0;
	std::string my_msg = "";
	// Receive data until the server closes the connection

	printf("\n[Type here]:");
	do {
		const int buflen = 512;
		uint8_t data[buflen];
		iResult = recv(ConnectSocket, (char*)&data[0], buflen, 0);
		if (iResult > 1)
		{
			Buffer buffer = Buffer(iResult);
			for (int i = 0; i < iResult; i++)
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
			std::string message = buffer.ReadString(msgSize);

			if (messageType == JOIN)
			{
				Client::roomName = roomName;
			}

			if (messageType == HELP)
			{
				std::cout << "\n\n-------HELP-------" << std::endl;

				std::cout << "\n[" << senderName << "] " << message << std::endl;
			}
			else {
				std::cout << "\n[" << roomName << "] " << "[" << senderName << "] " << message << std::endl;

			}
				if (Client::roomName.size() > 0)
					std::cout << "\n[" << Client::roomName << "] " << "[Type here]: " << my_msg;
				else
					std::cout << "\n[Type here]: " << my_msg;
		}

		if (iResult == 0)
		{
			printf("Connection closed\n");
			quit = 1;
		}

		char ch;
		if (_kbhit())
		{
			ch = _getch();
			if (ch == 27)
				quit = 1;
			if (ch >= 32 && ch <= 126)
			{
				my_msg += ch;
				std::cout << ch;
			}

			if (ch == 8)
			{
				if (my_msg.length() > 0)
					my_msg.pop_back();
				if(roomName.size() > 0)
					std::cout << "\n[" << roomName << "] " << "[Type here]: " << my_msg;
				else
					std::cout << "\n[Type here]: " << my_msg;
			}

			if (ch == 13 && my_msg.size() > 0)
			{
					
				if (my_msg == "quit")
					quit = 1;
				if (my_msg.substr(0, 5) == "_join")
				{
					if (my_msg.length() > 6)
					{
						std::string tempRoomName = my_msg.substr(6);
						printf("\nJoining %s\n", tempRoomName.c_str());
						MessagePacket packet;
						packet.header.messageType = JOIN;
						packet.content.senderName = clientName;
						packet.content.roomName = tempRoomName;
						packet.content.message = "joining";
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
						char* bufPtr = (char*)&(buffer.m_Buffer[0]);
						iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);
						if (iResult == SOCKET_ERROR)
						{
							// TODO: REMOVE THIS LATER
							printf("send failed: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}
					}
				}
				else if (my_msg == "_help")
				{
					MessagePacket packet;
					packet.header.messageType = HELP;
					packet.content.senderName = clientName;
					packet.content.roomName = roomName;
					packet.content.message = "help";
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
					char* bufPtr = (char*)&(buffer.m_Buffer[0]);
					iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);

					if (iResult == SOCKET_ERROR)
					{
						// TODO: REMOVE THIS LATER
						printf("send failed: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}
				}
				else if (my_msg == "_leave")
				{
					if (Client::roomName != "")
					{
						Client::roomName = "";
						MessagePacket packet;
						packet.header.messageType = LEAVE;
						packet.content.senderName = clientName;
						packet.content.roomName = roomName;
						packet.content.message = "leaving";
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
						char* bufPtr = (char*)&(buffer.m_Buffer[0]);
						iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);

						if (iResult == SOCKET_ERROR)
						{
							// TODO: REMOVE THIS LATER
							printf("send failed: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}
					}
				}
				else {
					MessagePacket packet;
					packet.header.messageType = MESSAGE;
					packet.content.senderName = clientName;
					packet.content.roomName = roomName;
					packet.content.message = my_msg;
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
					char* bufPtr = (char*)&(buffer.m_Buffer[0]);
					iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);

					if (iResult == SOCKET_ERROR)
					{
						// TODO: REMOVE THIS LATER
						printf("send failed: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}
				}

				my_msg.clear();
				if (Client::roomName.size() > 0)
					std::cout << "\n[" << Client::roomName << "] " << "[Type here]: " << my_msg;
				else
					std::cout << "\n[Type here]: " << my_msg;
			}
		}
		
		/*else
			printf("recv failed: %d\n", WSAGetLastError());*/
	} while (!quit);
	printf("Exit");

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int Client::Chatting()
{
	return 1;
}
