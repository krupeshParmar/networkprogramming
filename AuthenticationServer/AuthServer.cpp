#define WIN32_LEAN_AND_MEAN
#include "AuthServer.h"
#define DEFAULT_PORT "5551"
#define SERVER_NAME "GDP_AUTH_SERVER"
#include <iostream>
#include <string>
#include "Buffer.h"
#include "Protocol.h"

AuthServer::AuthServer()
{
	
}

AuthServer::~AuthServer()
{
	printf("Shutting Down Auth Server\n");
	closesocket(ListenSocket);
	WSACleanup();
}

int SendMessageTo(SOCKET socket, MessagePacket packet)
{
	Buffer buffer = Buffer(packet.header.packetLength);
	buffer.WriteInt32LE(packet.header.packetLength);
	buffer.WriteInt16LE(packet.header.messageType);
	buffer.WriteInt32LE(packet.content.senderName.size());
	buffer.WriteString(packet.content.senderName);
	buffer.WriteInt32LE(packet.content.roomName.size());
	buffer.WriteString(packet.content.roomName);
	buffer.WriteInt32LE(packet.content.message.size());
	buffer.WriteString(packet.content.message);
	if (packet.header.messageType < 5)
		return -2;
	int result = -2;
	try{
		result = send(socket, (const char*)&(buffer.m_Buffer[0]), packet.header.packetLength	, 0);
	}
	catch (std::exception e)
	{
		std::cout << e.what();
	}
	return result;
}

int AuthServer::Initialize()
{
	printf("\nStarting up GDP Auth Server\n");
	FD_ZERO(&activeSockets);
	//FD_ZERO(&socketReadyForReading);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	iResult = AuthServer::CreateSocket();
	if (iResult != 0)
	{
		printf("Socket creation failed: %d\n", iResult);
		return 1;
	}

	/*iResult = AuthServer::BindSocket();
	if (iResult != 0)
	{
		printf("Binding failed: %d\n", iResult);
		return 1;
	}

	iResult = AuthServer::Listen();
	if (iResult != 0)
	{
		printf("Listen failed: %d\n", iResult);
		return 1;
	}*/

	iResult = AuthServer::ConnectingSocket();
	if (iResult != 0)
	{
		return 1;
	}

	iResult = AuthServer::IOCtlSocket();
	if (iResult != 0)
	{
		return 1;
	}
	return iResult;
}

int AuthServer::Accept()
{
	return 0;
}

// Create listening socket
int AuthServer::CreateSocket()
{
	//printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		return 1;
	}
	ptr = result;
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		return 1;
	}
	return iResult;
}
//
//// Binding the socket to address
//int AuthServer::BindSocket()
//{
//	//printf("Calling BindSocket . . . ");
//	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//	if (iResult == SOCKET_ERROR)
//	{
//		printf("Bind failed with error: %d\n", WSAGetLastError());
//		freeaddrinfo(result);
//		closesocket(ListenSocket);
//		WSACleanup();
//		return 1;
//	}
//	freeaddrinfo(result);
//	return iResult;
//}
///// <summary>
///// Listen for connections
///// </summary>
///// <returns></returns>
//int AuthServer::Listen()
//{
//	//printf("Calling Listen . . . ");
//	iResult = listen(ListenSocket, SOMAXCONN);
//	if (iResult == SOCKET_ERROR)
//	{
//		printf("Listen failed with error: %ld\n", WSAGetLastError());
//		freeaddrinfo(result);
//		closesocket(ListenSocket);
//		WSACleanup();
//		return 1;
//	}
//	return 0;
//}
/// <summary>
/// Connect to server with given ip address
/// </summary>
/// <returns></returns>
int AuthServer::ConnectingSocket()
{
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
	return 0;
}

/// <summary>
/// Input output control fo socket
/// </summary>
/// <returns></returns>
int AuthServer::IOCtlSocket()
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

int AuthServer::ReceiveAndSend()
{
	MessagePacket packet;
	packet.header.messageType = MESSAGE;
	packet.content.senderName = "AUTH";
	packet.content.roomName = "AUTH";
	packet.content.message = "AUTH";
	packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
		+ 4 + packet.content.roomName.size()
		+ 4 + packet.content.message.size();

	// Serialize the message to send over the network
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

	while (1)
	{	// listen to connected clients
		const int buflen = DEFAULT_BUFLEN;
		uint8_t data[buflen];
		iResult = recv(ConnectSocket, (char*)&data[0], buflen, 0);			// try  to read the new messages
		if (iResult > 1)
		{
			chatserver.ChatSocket = ConnectSocket;
			Buffer buffer = Buffer(iResult);
			for (int i = 0; i < iResult; i++)
			{
				buffer.m_Buffer[i] = data[i];
			}
			std::string msg = "";
			int messagetype = -1;
			try {
				// Deserialize received message
				int packetlength = buffer.ReadInt32LE();
				messagetype = buffer.ReadInt16LE();
				int senderNameSize = buffer.ReadInt32LE();
				std::string senderName = buffer.ReadString(senderNameSize);
				int roomNameSize = buffer.ReadInt32LE();
				std::string roomName = buffer.ReadString(roomNameSize);
				int msgSize = buffer.ReadInt32LE();
				msg = buffer.ReadString(msgSize);
				int break_me = 0;
			}
			catch (std::exception e)
			{
				std::wcout << e.what() << std::endl;
				continue;
			}
			// Deserialize the message we received
			/*int packetlength = buffer.ReadInt32LE();
			int messageType = buffer.ReadInt16LE();
			int senderNameSize = buffer.ReadInt32LE();
			std::string senderName = buffer.ReadString(senderNameSize);
			int roomNameSize = buffer.ReadInt32LE();
			std::string roomName = buffer.ReadString(roomNameSize);
			int msgSize = buffer.ReadInt32LE();
			std::string message = buffer.ReadString(msgSize);*/
			if (messagetype == LOGIN)
			{
				authenticator::AuthenticateWeb login;
				if (login.ParseFromString(msg))
				{
					std::cout << "Email: " << login.email() << std::endl;
					std::cout << "Password: " << login.plaintextpassword() << std::endl;
					authenticator::AuthenticateWebSuccess logSucc;
					logSucc.set_requestid(login.requestid());
					logSucc.set_userid(123456);
					logSucc.set_creationdate("23/11/22");
					MessagePacket sendPacket;
					sendPacket.header.messageType = LOG_SUCC;
					sendPacket.content.roomName = SERVER_NAME;
					sendPacket.content.senderName = SERVER_NAME;
					logSucc.SerializeToString(&sendPacket.content.message);
					sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
						+ 4 + sendPacket.content.roomName.size()
						+ 4 + sendPacket.content.message.size();
					std::cout << std::to_string(logSucc.requestid()) << std::endl;
					iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
					std::cout << "Send result " << iSendResult << std::endl;
				}
				else std::cout << "Parsing failed" << std::endl;

			}
			if (messagetype == REGISTER)
			{
				authenticator::AuthenticateWeb login;
				if (login.ParseFromString(msg))
				{
					std::cout << "Email: " << login.email() << std::endl;
					std::cout << "Password: " << login.plaintextpassword() << std::endl;
				}
				else std::cout << "Parsing failed" << std::endl;
			}
			std::cout << msg << std::endl;
		}
	}
	return 0;
}
