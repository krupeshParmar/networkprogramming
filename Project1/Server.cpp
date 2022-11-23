#define WIN32_LEAN_AND_MEAN
#include "Server.h"
#define DEFAULT_PORT "5555"
#define SERVER_NAME "GDP SERVER"
#include <iostream>
#include <string>
#include <sstream>
#include "Buffer.h"
#include "Protocol.h"

Server::Server()
{
	helpString = "\nRooms you can join:\n\tgeneral\n\tresources\n\tpolls\n\tannouncements\n\toff-topic";
	helpString += "\nUse _join room_name command to join a room\n";
	helpString += "\nUse _send room_name message command to send message to the room\n";
	helpString += "\nUse _leave command to leave the room\n";
	helpString += "\nUse _help command to get help\n";
	helpString += "\nUse _quit command to exit\n";
}

// close socket
Server::~Server()
{
	printf("Shutting down\n");
	closesocket(ListenSocket);
	WSACleanup();
}

int SendMessageTo(Client client, MessagePacket packet)
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

	return send(client.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
}

/// <summary>
/// Initialize the Winsock connection
/// </summary>
/// <returns></returns>
int Server::Initialize()
{
	printf("\nStarting up the GDP Server\n");

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

// Create listening socket
int Server::CreateSocket()
{
	//printf("Calling Create Socket . . . ");
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

// Binding the socket to address
int Server::BindSocket()
{
	//printf("Calling BindSocket . . . ");
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
/// <summary>
/// Listen for connections
/// </summary>
/// <returns></returns>
int Server::Listen()
{
	//printf("Calling Listen . . . ");
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

/// <summary>
/// Accept new connections
/// </summary>
/// <returns></returns>
int Server::Accept()
{
	//printf("Calling Accept . . . ");
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
		// If a new client is connected, then read their name and send them a warm welcome
		const int buflen = 128;
		uint8_t buf[buflen];

		Client client;
		client.clientSocket = clientSocket;
		client.connected = true;

		int recvResult = recv(clientSocket, (char*)&buf, buflen, 0);
		if (recvResult > 0)
		{
			Buffer buffer = Buffer(recvResult);
			for (int i = 0; i < recvResult; i++)
			{
				buffer.m_Buffer[i] = buf[i];
			}
			// Deserialize message to get client's name
			int packetlength = buffer.ReadInt32LE();
			int messageType = buffer.ReadInt16LE();
			int senderNameSize = buffer.ReadInt32LE();
			std::string senderName = buffer.ReadString(senderNameSize);
			int roomNameSize = buffer.ReadInt32LE();
			std::string roomName = buffer.ReadString(roomNameSize);
			int msgSize = buffer.ReadInt32LE();
			std::string msg = buffer.ReadString(msgSize);
			client.clientName = msg;
			client.authenticated = false;
			if (client.clientName == "AUTH")
			{
				std::cout << "Encountered auth" << std::endl;
				authenticatorServer.AuthSocket = clientSocket; 
				authenticator::AuthenticateWeb auth;
				auth.set_requestid(time(NULL));
				auth.set_plaintextpassword("password");
				auth.set_email("user@fanshawe.ca");
				MessagePacket packet;
				std::cout << client.clientName << " connected" << std::endl;
				packet.header.messageType = MESSAGE;
				packet.content.roomName = SERVER_NAME;
				packet.content.senderName = SERVER_NAME;
				packet.content.message = "Welcome";

				packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
					+ 4 + packet.content.roomName.size()
					+ 4 + packet.content.message.size();

				// Serialize message
				Buffer buffer = Buffer(packet.header.packetLength);
				buffer.WriteInt32LE(packet.header.packetLength);
				buffer.WriteInt16LE(packet.header.messageType);
				buffer.WriteInt32LE(packet.content.senderName.size());
				buffer.WriteString(packet.content.senderName);
				buffer.WriteInt32LE(packet.content.roomName.size());
				buffer.WriteString(packet.content.roomName);
				buffer.WriteInt32LE(packet.content.message.size());
				buffer.WriteString(packet.content.message);
				// Send the welcome message to the client
				iSendResult = send(authenticatorServer.AuthSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
				authenticatorServer.connected = true;
				return 0;
			}
			if (msg == client.clientName)
			{
				MessagePacket packet;
				std::cout << client.clientName << " connected" << std::endl;
				packet.header.messageType = WELCOME;
				packet.content.roomName = "";
				packet.content.senderName = SERVER_NAME;
				packet.content.message = "Welcome " + client.clientName + helpString;

				packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
					+ 4 + packet.content.roomName.size()
					+ 4 + packet.content.message.size();

				// Serialize message
				Buffer buffer = Buffer(packet.header.packetLength);
				buffer.WriteInt32LE(packet.header.packetLength);
				buffer.WriteInt16LE(packet.header.messageType);
				buffer.WriteInt32LE(packet.content.senderName.size());
				buffer.WriteString(packet.content.senderName);
				buffer.WriteInt32LE(packet.content.roomName.size());
				buffer.WriteString(packet.content.roomName);
				buffer.WriteInt32LE(packet.content.message.size());
				buffer.WriteString(packet.content.message);
				// Send the welcome message to the client
				iSendResult = send(client.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
			}
		}
		// add the client to clients list
		clients.push_back(client);
	}
	return 0;
}

int Server::GetRoomId(std::string roomName)
{
	int id = -1;
	for (int i = 0; i < TOTAL_ROOMS; i++)
	{
		if (Rooms[i] == roomName)
		{
			id = i;
			break;
		}
	}
	return id;
}


int Server::AuthenticateUser(authenticator::AuthenticateWeb* auth)
{
	MessagePacket packet;
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	packet.content.message = auth->SerializeAsString();
	std::cout << packet.content.message;

	packet.header.messageType = LOGIN;
	packet.content.roomName = SERVER_NAME;
	packet.content.senderName = SERVER_NAME;

	packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
		+ 4 + packet.content.roomName.size()
		+ 4 + packet.content.message.size();

	// Serialize message
	Buffer buffer = Buffer(packet.header.packetLength);
	buffer.WriteInt32LE(packet.header.packetLength);
	buffer.WriteInt16LE(packet.header.messageType);
	buffer.WriteInt32LE(packet.content.senderName.size());
	buffer.WriteString(packet.content.senderName);
	buffer.WriteInt32LE(packet.content.roomName.size());
	buffer.WriteString(packet.content.roomName);
	buffer.WriteInt32LE(packet.content.message.size());
	buffer.WriteString(packet.content.message);
	// Send the welcome message to the client
	iSendResult = send(authenticatorServer.AuthSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);

	return iSendResult;
}
int Server::AuthenticateUser(authenticator::CreateAccountWeb* auth)
{
	MessagePacket packet;
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	packet.content.message = auth->SerializeAsString();
	std::cout << packet.content.message;

	packet.header.messageType = REGISTER;
	packet.content.roomName = SERVER_NAME;
	packet.content.senderName = SERVER_NAME;

	packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
		+ 4 + packet.content.roomName.size()
		+ 4 + packet.content.message.size();

	// Serialize message
	Buffer buffer = Buffer(packet.header.packetLength);
	buffer.WriteInt32LE(packet.header.packetLength);
	buffer.WriteInt16LE(packet.header.messageType);
	buffer.WriteInt32LE(packet.content.senderName.size());
	buffer.WriteString(packet.content.senderName);
	buffer.WriteInt32LE(packet.content.roomName.size());
	buffer.WriteString(packet.content.roomName);
	buffer.WriteInt32LE(packet.content.message.size());
	buffer.WriteString(packet.content.message);
	// Send the welcome message to the client
	iSendResult = send(authenticatorServer.AuthSocket, (const char*)&(buffer.m_Buffer[0]), packet.header.packetLength, 0);

	return iSendResult;
}

/// <summary>
/// Broadcast the message to proper clients
/// </summary>
/// <param name="msg"> Message to be sent </param>
/// <param name="roomName"> Name of room where the message is supposed to go </param>
/// <param name="sender"> Client who is sending the message </param>
/// <param name="messageType"> Type of message </param>
/// <returns></returns>
int Server::Broadcast(std::string msg, std::string roomName, Client& sender, int messageType)
{
	// print the message to the server first
	std::cout << "[ "  << roomName << "] " << "[ " << sender.clientName << "] " << msg << std::endl;

	// if message type is HELP, send help text
	if (messageType == HELP)
	{
		MessagePacket packet;
		packet.content.message = helpString;
		packet.header.messageType = HELP;
		packet.content.roomName = roomName;
		packet.content.senderName = SERVER_NAME;
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

	// search whether the provided room exists
	for (int i = 0; i < TOTAL_ROOMS; i++)
		if (roomName == Rooms[i])
			roomFound = true;

	// If room exists...
	if (roomFound)
	{
		int roomID = GetRoomId(roomName);
		// If message type is join, add the client to the room, and send them a welcome message
		if (messageType == JOIN)
		{
			if(sender.rooms[roomID])
			{
				MessagePacket packet;

				packet.header.messageType = MESSAGE;
				packet.content.roomName = roomName;
				packet.content.senderName = sender.clientName;
				packet.content.message = "\nAlready joined " + roomName + "\n";
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
			}
			else {
				MessagePacket packet;

				packet.header.messageType = JOIN;
				packet.content.roomName = roomName;
				packet.content.senderName = sender.clientName;
				packet.content.message = "\nWelcome to " + roomName + "\n";
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

				sender.rooms[roomID] = 1;
				iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
			}
			
		}

		// Broadcast the message to all the clients in given room
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i].rooms[roomID])
			{
				MessagePacket packet;
				std::cout << msg;
				packet.header.messageType = MESSAGE;
				packet.content.roomName = roomName;
				packet.content.senderName = sender.clientName;
				packet.content.message = msg;
				packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
					+ 4 + packet.content.roomName.size()
					+ 4 + packet.content.message.size();

				// Serialize the packet
				Buffer buffer = Buffer(packet.header.packetLength);
				buffer.WriteInt32LE(packet.header.packetLength);
				buffer.WriteInt16LE(packet.header.messageType);
				buffer.WriteInt32LE(packet.content.senderName.size());
				buffer.WriteString(packet.content.senderName);
				buffer.WriteInt32LE(packet.content.roomName.size());
				buffer.WriteString(packet.content.roomName);
				buffer.WriteInt32LE(packet.content.message.size());
				buffer.WriteString(packet.content.message);

				// Send the packet to room members
				iSendResult = send(clients[i].clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
			}
		}

		// if the message type is LEAVE, remove the client from the room
		if (messageType == LEAVE)
		{
			sender.rooms[roomID] = 0;
			MessagePacket packet;
			packet.header.messageType = LEAVE;
			packet.content.roomName = roomName;
			packet.content.senderName = SERVER_NAME;
			packet.content.message = "\nYou have left "+roomName+"\n";
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
			// Send the packet to room members
			iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), DEFAULT_BUFLEN, 0);
		}
		return iSendResult;
	}
	else {		// If the room not found ...
		MessagePacket packet;
		packet.header.messageType = MESSAGE;
		packet.content.roomName = "";
		packet.content.senderName = SERVER_NAME;

		// If the message type is join, tell client to enter a valid room name
		if (messageType == JOIN || messageType == LEAVE)
		{
			packet.content.message = "No group named " + roomName + " exists.";
			packet.content.message += "\n Use _help command to know about the server\n";
		}
		else {
			packet.content.message = "\nPlease input proper command\n";
			packet.content.message += "\n Use _help command to know about the server\n";
		}
		packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
			+ 4 + packet.content.roomName.size()
			+ 4 + packet.content.message.size();

		// Serialize the packet
		Buffer buffer = Buffer(packet.header.packetLength);
		buffer.WriteInt32LE(packet.header.packetLength);
		buffer.WriteInt16LE(packet.header.messageType);
		buffer.WriteInt32LE(packet.content.senderName.size());
		buffer.WriteString(packet.content.senderName);
		buffer.WriteInt32LE(packet.content.roomName.size());
		buffer.WriteString(packet.content.roomName);
		buffer.WriteInt32LE(packet.content.message.size());
		buffer.WriteString(packet.content.message);

		// send the serialized packet
		iSendResult = send(sender.clientSocket, (const char*)&(buffer.m_Buffer[0]), packet.header.packetLength, 0);
		return iSendResult;
	}	
}

/// <summary>
/// Send and receive messages to and from the clients
/// </summary>
/// <returns></returns>
int Server::ReceiveAndSend()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;

	while (1)
	{
		FD_ZERO(&socketsReadyForReading);

		FD_SET(ListenSocket, &socketsReadyForReading);

		// listen to connected clients
		for (int i = 0; i < clients.size(); i++)
		{
			Client& client = clients[i];
			if (client.connected)
			{
				FD_SET(client.clientSocket, &socketsReadyForReading);
			}
		}

		if (authenticatorServer.connected)
			FD_SET(authenticatorServer.AuthSocket, &socketsReadyForReading);

		// perform synchronous I/O
		iSelectResult = select(0, &socketsReadyForReading, NULL, NULL, &tv);
		if (iSelectResult == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			return 1;
		}
		printf(".");

		// If ther is a new connection then accept it
		if (FD_ISSET(ListenSocket, &socketsReadyForReading))
		{
			printf("\n");
			Server::Accept();
			continue;
		}

		/*if (FD_ISSET(authenticatorServer.AuthSocket, &socketsReadyForReading))
		{
			const int buflen = 128;
			uint8_t data[buflen];

			int recvResult = recv(authenticatorServer.AuthSocket, (char*)&data[0], buflen, 0);

			if (recvResult == 0)
			{
				std::cout << "Authenticator disconnected" << std::endl;
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

				// Deserialize received message
				int packetlength = buffer.ReadInt32LE();
				int messageType = buffer.ReadInt16LE();
				int senderNameSize = buffer.ReadInt32LE();
				std::string senderName = buffer.ReadString(senderNameSize);
				int roomNameSize = buffer.ReadInt32LE();
				std::string roomName = buffer.ReadString(roomNameSize);
				int msgSize = buffer.ReadInt32LE();
				std::string msg = buffer.ReadString(msgSize);
				std::cout << msg << std::endl;
				
				// If send failed, print "Error"
				if (sendResult < 0)
					printf("\nError\n");
			}
		}*/
		if (authenticatorServer.AuthSocket != INVALID_SOCKET)
		{
			if (FD_ISSET(authenticatorServer.AuthSocket, &socketsReadyForReading))
			{
				const int buflen = 128;
				uint8_t data[buflen];

				int recvResult = recv(authenticatorServer.AuthSocket, (char*)&data[0], buflen, 0);

				if (recvResult == 0)
				{
					std::cout << "Authenticator disconnected" << std::endl;
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
					/*try {*/
						// Deserialize received message
					int packetlength = buffer.ReadInt32LE();
					int messageType = buffer.ReadInt16LE();
					int senderNameSize = buffer.ReadInt32LE();
					std::string senderName = buffer.ReadString(senderNameSize);
					int roomNameSize = buffer.ReadInt32LE();
					std::string roomName = buffer.ReadString(roomNameSize);
					int msgSize = buffer.ReadInt32LE();
					std::string message = buffer.ReadString(msgSize);
					int break_me = 0;
					/*}
					catch (std::exception e)
					{
						std::wcout << e.what() << std::endl;
						continue;
					}*/
					authenticator::CreateAccountWebFailure regFail;
					authenticator::CreateAccountWebSuccess regSucc;
					authenticator::AuthenticateWebSuccess logSucc;
					authenticator::AuthenticateWebFailure logFail;

					MessagePacket packet;
					switch (messageType)
					{
					case REG_FAIL:
						regFail.ParseFromString(message);
						for (Client client : clients)
						{
							if (client.requestID == regFail.requestid())
							{
								packet.header.messageType = REG_FAIL;
								packet.content.roomName = SERVER_NAME;
								packet.content.senderName = SERVER_NAME;
								packet.content.message = "\nFailed to register account";
								packet.content.message += "\nBecause: ";
								switch (regFail.failreason())
								{
									case
									authenticator::CreateAccountWebFailure_reason::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS:
										packet.content.message += "Account already exists";
										break;
										case
										authenticator::CreateAccountWebFailure_reason::CreateAccountWebFailure_reason_INVALID_PASSWORD:
											packet.content.message += "Invalid credentials";
											break;
											case
											authenticator::CreateAccountWebFailure_reason::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR:
												packet.content.message += "Internal Server Error";
												break;
											default:
												break;
								}
								packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
									+ 4 + packet.content.roomName.size()
									+ 4 + packet.content.message.size();
								iSendResult = SendMessageTo(client, packet);
							}
						}
						break;
					case REG_SUCC:
						regSucc.ParseFromString(message);
						for (Client client : clients)
						{
							if (client.requestID == regSucc.requestid())
							{
								packet.header.messageType = REG_SUCC;
								packet.content.roomName = SERVER_NAME;
								packet.content.senderName = SERVER_NAME;
								packet.content.message = "\nAccount created succesfully";
								packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
									+ 4 + packet.content.roomName.size()
									+ 4 + packet.content.message.size();
								iSendResult = SendMessageTo(client, packet);
							}
						}
						break;
					case LOG_FAIL:
						logFail.ParseFromString(message);
						for (Client client : clients)
						{
							if (client.requestID == logFail.requestid())
							{
								packet.header.messageType = LOG_FAIL;
								packet.content.roomName = SERVER_NAME;
								packet.content.senderName = SERVER_NAME;
								packet.content.message = "\nFailed to login";
								packet.content.message += "\nBecause: ";
								switch (logFail.failreason())
								{
									case
									authenticator::AuthenticateWebFailure_reason::AuthenticateWebFailure_reason_INVALID_CREDENTIALS:
										packet.content.message += "Invalid credentials";
										break;
									case authenticator::AuthenticateWebFailure_reason::AuthenticateWebFailure_reason_INTERNAL_SERVER_ERROR:
										packet.content.message += "Internal Server Error";
										break;
									default:
										break;
								}
								packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
									+ 4 + packet.content.roomName.size()
									+ 4 + packet.content.message.size();
								iSendResult = SendMessageTo(client, packet);
							}
						}
						break;
					case LOG_SUCC:
						logSucc.ParseFromString(message);
						for (Client client : clients)
						{
							if (client.requestID == logSucc.requestid())
							{
								packet.header.messageType = LOG_SUCC;
								packet.content.roomName = SERVER_NAME;
								packet.content.senderName = SERVER_NAME;
								packet.content.message = "\nLogged in successfully";
								packet.header.packetLength = 8 + 4 + packet.content.senderName.size()
									+ 4 + packet.content.roomName.size()
									+ 4 + packet.content.message.size();
								iSendResult = SendMessageTo(client, packet);
							}
						}
						break;
					}
				}
			}
		}
		// Look for new messages from all the connected clients
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
					std::cout << client.clientName << " disconnected" << std::endl;
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
					
					// Deserialize received message
					int packetlength = buffer.ReadInt32LE();
					int messageType = buffer.ReadInt16LE();
					int senderNameSize = buffer.ReadInt32LE();
					std::string senderName = buffer.ReadString(senderNameSize);
					int roomNameSize = buffer.ReadInt32LE();
					std::string roomName = buffer.ReadString(roomNameSize);
					int msgSize = buffer.ReadInt32LE();
					std::string msg = buffer.ReadString(msgSize);
					std::cout << msg << std::endl;

					authenticator::AuthenticateWeb loginAuth;
					authenticator::CreateAccountWeb registerAuth;
					// broadcast message, depending on the message type
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
					case LOGIN:
						loginAuth.ParseFromString(msg);
						client.email = loginAuth.email();
						client.requestID = loginAuth.requestid();
						std::cout << std::to_string(loginAuth.requestid()) << std::endl;
						sendResult = this->AuthenticateUser(&loginAuth);
						break;
					case REGISTER:
						registerAuth.ParseFromString(msg);
						client.email = registerAuth.email();
						client.requestID = registerAuth.requestid();
						sendResult = this->AuthenticateUser(&registerAuth);
						break;
					default:
						break;
					}

					// If send failed, print "Error"
					if (sendResult < 0)
						printf("\nError\n");
				}
			}
		}
	}
}
