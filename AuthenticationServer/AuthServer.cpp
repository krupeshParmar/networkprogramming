#define WIN32_LEAN_AND_MEAN
#include "AuthServer.h"
#define DEFAULT_PORT "5555"
#define SERVER_NAME "GDP_AUTH_SERVER"
#include <iostream>
#include <string>
#include "Buffer.h"
#include "Protocol.h"
#include <pugixml.hpp>
#include "sha256.h"

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


int AuthServer::LoadUsers()
{
	pugi::xml_document doc;
	if (!doc.load_file("userdata/userdata.xml"))
	{
		std::cout << "Couldn't open the file\n";
		return -1;
	}
	pugi::xml_object_range<pugi::xml_node_iterator> usersNode
		= doc.child("users").children();
	pugi::xml_node_iterator usersIterator = usersNode.begin();
	for (; usersIterator != usersNode.end(); usersIterator++)
	{
		pugi::xml_node userNode = *usersIterator;

		std::string nodeName = userNode.name();
		if (nodeName == "user")
		{
			User* user = new User();
			pugi::xml_object_range<pugi::xml_node_iterator> userData = userNode.children();
			for (
				pugi::xml_node_iterator userDataIterator = userData.begin(); 
				userDataIterator != userData.end(); 
				userDataIterator++)
			{
				pugi::xml_node userDataNode = *userDataIterator;

				std::string userDataName = userDataNode.name();
				if (userDataName == "id")
				{
					user->ID = std::stol(userDataNode.child_value());
				}
				if (userDataName == "email")
				{
					user->email = userDataNode.child_value();
				}
				if (userDataName == "salt")
				{
					user->salt = userDataNode.child_value();
				}
				if (userDataName == "hashedPassword")
				{
					user->hashedPassword = userDataNode.child_value();
				}
				if (userDataName == "userID")
				{
					user->userID = std::stol(userDataNode.child_value());
				}
				if (userDataName == "last_login")
				{
					user->last_login = userDataNode.child_value();
				}
				if (userDataName == "creationDate")
				{
					user->creationDate = userDataNode.child_value();
				}
			}
			this->users.push_back(user);
		}
	}
}
int AuthServer::SaveUsers()
{
	pugi::xml_document doc;
	pugi::xml_node saveNode = doc.append_child("users");
	for (int i = 0; i < users.size(); i++)
	{
		pugi::xml_node userNode = saveNode.append_child("user");

		pugi::xml_node emailNode = userNode.append_child("email");
		emailNode.append_child(pugi::node_pcdata).set_value(this->users[i]->email.c_str());

		pugi::xml_node idNode = userNode.append_child("id");
		idNode.append_child(pugi::node_pcdata).set_value(std::to_string(this->users[i]->ID).c_str());

		pugi::xml_node saltlNode = userNode.append_child("salt");
		saltlNode.append_child(pugi::node_pcdata).set_value(this->users[i]->salt.c_str());

		pugi::xml_node hashedPasswordNode = userNode.append_child("hashedPassword");
		hashedPasswordNode.append_child(pugi::node_pcdata).set_value(this->users[i]->hashedPassword.c_str());

		pugi::xml_node userIDNode = userNode.append_child("userID");
		userIDNode.append_child(pugi::node_pcdata).set_value(std::to_string(this->users[i]->userID).c_str());

		pugi::xml_node last_loginNode = userNode.append_child("last_login");
		last_loginNode.append_child(pugi::node_pcdata).set_value(this->users[i]->last_login.c_str());

		pugi::xml_node creationDateNode = userNode.append_child("creationDate");
		creationDateNode.append_child(pugi::node_pcdata).set_value(this->users[i]->creationDate.c_str());
	}
	doc.save_file("userdata/userdata.xml");
	return 1;
}
int AuthServer::CreateUser(std::string email, std::string password, long int requestId)
{
	for (User* user : users)
	{
		if (user->email == email)
		{
			// Account already exists
			authenticator::CreateAccountWebFailure regiFailure;
			regiFailure.set_failreason(authenticator::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS);
			regiFailure.set_requestid(requestId);
			MessagePacket sendPacket;
			sendPacket.header.messageType = REG_FAIL;
			sendPacket.content.roomName = SERVER_NAME;
			sendPacket.content.senderName = SERVER_NAME;
			regiFailure.SerializeToString(&sendPacket.content.message);
			sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
				+ 4 + sendPacket.content.roomName.size()
				+ 4 + sendPacket.content.message.size();

			iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
			return -1;
		}
	}
	// No account exist with "email"

	srand(time(NULL));
	authenticator::CreateAccountWeb regi;
	User* user = new User();
	user->email = email;
	std::string salt = std::to_string((rand() % 1000000 + 1000000));
	user->salt = salt;
	SHA256 sha;
	user->hashedPassword = sha(password + salt);
	user->ID = rand() % 5000 + 1000;
	user->last_login = std::to_string(time(NULL));
	user->userID = time(NULL);
	users.push_back(user);
	if (this->SaveUsers() != 1)
	{
		authenticator::CreateAccountWebFailure regiFailure;
		regiFailure.set_failreason(authenticator::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR);
		regiFailure.set_requestid(requestId);
		MessagePacket sendPacket;
		sendPacket.header.messageType = REG_FAIL;
		sendPacket.content.roomName = SERVER_NAME;
		sendPacket.content.senderName = SERVER_NAME;
		regiFailure.SerializeToString(&sendPacket.content.message);
		sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
			+ 4 + sendPacket.content.roomName.size()
			+ 4 + sendPacket.content.message.size();

		iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
		return -1;
	}
	authenticator::CreateAccountWebSuccess regiSucc;
	regiSucc.set_userid(user->userID);
	regiSucc.set_requestid(requestId);
	MessagePacket sendPacket;
	sendPacket.header.messageType = REG_SUCC;
	sendPacket.content.roomName = SERVER_NAME;
	sendPacket.content.senderName = SERVER_NAME;
	regiSucc.SerializeToString(&sendPacket.content.message);
	sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
		+ 4 + sendPacket.content.roomName.size()
		+ 4 + sendPacket.content.message.size();

	iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
	return 0;
}
int AuthServer::AuthenticateUser(std::string email, std::string password, long int requestId)
{
	for (User* user : users)
	{
		if (user->email == email)
		{
			SHA256 sha;
			std::string hashedPassword = sha(password + user->salt);
			if (user->hashedPassword == hashedPassword)
			{
				authenticator::AuthenticateWebSuccess loginSuccess;
				loginSuccess.set_creationdate("23/11");
				loginSuccess.set_userid(user->userID);
				loginSuccess.set_requestid(requestId);
				MessagePacket sendPacket;
				sendPacket.header.messageType = LOG_SUCC;
				sendPacket.content.roomName = SERVER_NAME;
				sendPacket.content.senderName = SERVER_NAME;
				loginSuccess.SerializeToString(&sendPacket.content.message);
				sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
					+ 4 + sendPacket.content.roomName.size()
					+ 4 + sendPacket.content.message.size();

				iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
			}
			else {
				authenticator::AuthenticateWebFailure loginFailure;
				loginFailure.set_failreason(authenticator::AuthenticateWebFailure_reason_INVALID_CREDENTIALS);
				loginFailure.set_requestid(requestId);
				MessagePacket sendPacket;
				sendPacket.header.messageType = LOG_FAIL;
				sendPacket.content.roomName = SERVER_NAME;
				sendPacket.content.senderName = SERVER_NAME;
				loginFailure.SerializeToString(&sendPacket.content.message);
				sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
					+ 4 + sendPacket.content.roomName.size()
					+ 4 + sendPacket.content.message.size();

				iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
			}
			return 1;
		}
	}
	authenticator::AuthenticateWebFailure loginFailure;
	loginFailure.set_failreason(authenticator::AuthenticateWebFailure_reason_INVALID_CREDENTIALS);
	loginFailure.set_requestid(requestId);
	MessagePacket sendPacket;
	sendPacket.header.messageType = LOG_FAIL;
	sendPacket.content.roomName = SERVER_NAME;
	sendPacket.content.senderName = SERVER_NAME;
	loginFailure.SerializeToString(&sendPacket.content.message);
	sendPacket.header.packetLength = 8 + 4 + sendPacket.content.senderName.size()
		+ 4 + sendPacket.content.roomName.size()
		+ 4 + sendPacket.content.message.size();

	iSendResult = SendMessageTo(chatserver.ChatSocket, sendPacket);
	return -1;
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
	this->LoadUsers();
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
				std::cout << e.what() << std::endl;
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
					this->AuthenticateUser(login.email(), login.plaintextpassword(), login.requestid());
				}
				else std::cout << "Parsing failed" << std::endl;

			}
			if (messagetype == REGISTER)
			{
				authenticator::CreateAccountWeb regi;
				if (regi.ParseFromString(msg))
				{
					this->CreateUser(regi.email(), regi.plaintextpassword(), regi.requestid());
				}
				else std::cout << "Parsing failed" << std::endl;
			}
			std::cout << msg << std::endl;
		}
	}
	return 0;
}
