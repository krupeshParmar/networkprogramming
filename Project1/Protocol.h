#pragma once
#include <string>

enum MessageType
{
	JOIN,
	MESSAGE,
	LEAVE,
	WELCOME,
	HELP,
};

struct PacketHeader
{
	int packetLength;
	int messageType;
};

struct MessageData
{
	std::string roomName;
	std::string message;
	std::string senderName;
};

struct MessagePacket
{
	PacketHeader header;
	MessageData content;
};
