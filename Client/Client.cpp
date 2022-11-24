#include "Client.h"
#include <iostream>
#include <string>
#include <conio.h>
#include <sstream>
#include "Buffer.h"
#include "Protocol.h"

#define DEFAULT_PORT "5555"

// Initialize with empty room
Client::Client()
{

}

// shutdown socket
Client::~Client()
{
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
	}
	closesocket(ConnectSocket);
	WSACleanup();
}

/// <summary>
/// Initialize winsock connection
/// </summary>
/// <param name="ipadd"></param>
/// <returns></returns>
int Client::Initialize(const char* ipadd)
{
	printf("Trying to connect with GDP server");
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

/// <summary>
/// Create a socket
/// </summary>
/// <param name="ipadd"></param>
/// <returns></returns>
int Client::CreateSocket(const char* ipadd)
{
	//printf("Calling Create Socket . . . ");
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ipadd, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	return 0;
}

/// <summary>
/// Connect to server with given ip address
/// </summary>
/// <returns></returns>
int Client::ConnectingSocket()
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

int Client::GetRoomId(std::string roomName)
{
	int id = -1;
	for (int i = 0; i < 5; i++)
	{
		if (Rooms[i] == roomName)
		{
			id = i;
			break;
		}
	}
	return id;
}

/// <summary>
/// Send and receive messages
/// </summary>
/// <returns></returns>
int Client::SendAndReceive()
{
	// Send an initial message, i.e., your name
	MessagePacket packet;
	packet.header.messageType = MESSAGE;
	packet.content.senderName = clientName;
	packet.content.roomName = "";
	packet.content.message = clientName;
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

	// chat loop
	printf("\n Message: ");
	do {
		const int buflen = DEFAULT_BUFLEN;
		uint8_t data[buflen];
		iResult = recv(ConnectSocket, (char*)&data[0], buflen, 0);			// try  to read the new messages
		if (iResult > 1)
		{
			Buffer buffer = Buffer(iResult);
			for (int i = 0; i < iResult; i++)
			{
				buffer.m_Buffer[i] = data[i];
			}

			// Deserialize the message we received
			int packetlength = buffer.ReadInt32LE();
			int messageType = buffer.ReadInt16LE();
			int senderNameSize = buffer.ReadInt32LE();
			std::string senderName = buffer.ReadString(senderNameSize);
			int roomNameSize = buffer.ReadInt32LE();
			std::string roomName = buffer.ReadString(roomNameSize);
			int msgSize = buffer.ReadInt32LE();
			std::string message = buffer.ReadString(msgSize);

			int roomid = Client::GetRoomId(roomName);
			if (messageType == JOIN)
			{
				roomsJoined[roomid] = 1;
			}

			if (messageType == LEAVE)
			{
				roomsJoined[roomid] = 0;
			}

			// display  the received messages
			if (messageType == HELP)
			{
				std::cout << "\n\n-------HELP-------" << std::endl;

				std::cout << "\n[" << senderName << "] " << message << std::endl;
			}
			else {
				std::cout << "\n[" << roomName << "] " << "[" << senderName << "] " << message << std::endl;
			}
			std::cout << "\n Message: " << my_msg;
		}

		// if we receive zero, then connection  has been closed by server
		if (iResult == 0)
		{
			printf("Connection closed\n");
			quit = 1;
		}

		// take input from keyboard
		char ch;
		if (_kbhit())
		{
			ch = _getch();
			if (ch == 27)	// Escape key to quit the app
				quit = 1;

			if (ch >= 32 && ch <= 126)	// If the character pressed is valid then add them to your my_msg string and display
			{
				my_msg += ch;
				std::cout << ch;
			}

			if (ch == 8)		// On backspace key, remove the last char, if there is at least one char in my_msg
			{
				if (my_msg.length() > 0)
					my_msg.pop_back();
				std::cout << "\n Message: " << my_msg;
			}

			if (ch == 13 && my_msg.size() > 0)		// If enter is hit, and my_msg has at least one element
			{
				if (my_msg.substr(0, 6) == "_login")
				{
					std::string email, password;
					int sep = -1;

					for (int j = 7; j < my_msg.size(); j++)
					{
						if (my_msg[j] == ' ')
						{
							sep = j;
							break;
						}
						email += my_msg[j];
					}
					password = my_msg.substr(sep + 1);
					authenticator::AuthenticateWeb login;
					login.set_email(email);
					login.set_plaintextpassword(password);
					login.set_requestid(time(NULL));
					MessagePacket packet;
					packet.header.messageType = LOGIN;
					packet.content.senderName = clientName;
					packet.content.roomName = "";
					login.SerializeToString(&packet.content.message);
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
					iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);	// send the message

				}
				
				if (my_msg.substr(0, 9) == "_register")
				{
					std::string email, password;
					int sep = -1;

					for (int j = 10; j < my_msg.size(); j++)
					{
						if (my_msg[j] == ' ')
						{
							sep =	 j;
							break;
						}
						email += my_msg[j];
					}
					password = my_msg.substr(sep + 1);
					authenticator::CreateAccountWeb regis;
					regis.set_email(email);
					regis.set_plaintextpassword(password);
					regis.set_requestid(time(NULL));
					MessagePacket packet;
					packet.header.messageType = REGISTER;
					packet.content.senderName = clientName;
					packet.content.roomName = "";
					regis.SerializeToString(&packet.content.message);
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
					iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);	// send the message

				}
				if (my_msg == "_quit")				// If the message is "quit" then exit the app
					quit = 1;
				if (my_msg.substr(0, 5) == "_join")	// If the message is "_join" then try to join the provided room
				{
					if (my_msg.length() > 6)
					{
						std::string tempRoomName = my_msg.substr(6);	// get room name
						printf("\nJoining %s\n", tempRoomName.c_str());
						MessagePacket packet;
						packet.header.messageType = JOIN;
						packet.content.senderName = clientName;
						packet.content.roomName = tempRoomName;
						packet.content.message = "joining";
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
						iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);	// send the message

						// if message not sent, display failed attempt
						if (iResult == SOCKET_ERROR)
						{
							printf("send failed: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}
					}
				}
				else if (my_msg == "_help")		// If the message is "_help" then try to get help from server
				{
					MessagePacket packet;
					packet.header.messageType = HELP;
					packet.content.senderName = clientName;
					packet.content.roomName = "";
					packet.content.message = "help";
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
					iResult = send(ConnectSocket, bufPtr, packet.header.packetLength, 0);		// send the message

					// if message not sent, display failed attempt
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}
				}
				else if (my_msg.substr(0, 6) == "_leave")		// If the message is "_leave" then try to leave the current room
				{
					if (my_msg.length() > 7)
					{
						std::string tempRoomName = my_msg.substr(7);

						MessagePacket packet;
						packet.header.messageType = LEAVE;
						packet.content.senderName = clientName;
						packet.content.roomName = tempRoomName;
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
							printf("send failed: %d\n", WSAGetLastError());
							closesocket(ConnectSocket);
							WSACleanup();
							return 1;
						}
					}
				}
				else if(my_msg.substr(0, 5) == "_send")		// If it is just a normal message then try to send it to the current room
				{
					if (my_msg.length() > 6)
					{
						std::string command = my_msg.substr(6);
						std::stringstream ss(command);
						std::string roomName;
						ss >> roomName;
						if (roomsJoined[Client::GetRoomId(roomName)])
						{
							//std::cout << roomName << "\n";
							std::string message = "";
							std::string words;

							while (ss >> words)
								message += words + " ";
							MessagePacket packet;
							packet.header.messageType = MESSAGE;
							packet.content.senderName = clientName;
							packet.content.roomName = roomName;
							packet.content.message = message;
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
								printf("send failed: %d\n", WSAGetLastError());
								closesocket(ConnectSocket);
								WSACleanup();
								return 1;
							}
						}
						else {
							std::cout << "\n" << roomName << " not joined\n";
						}
						
					}
					else
					{
						std::cout << "\nInvalid commands\n";
					}
				}

				my_msg.clear();
				std::cout << "\n Message: " << my_msg;
			}
		}
	} while (!quit);	// If not quit, continue the loop
	printf("Exit");

	// Shutdown the connection
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

