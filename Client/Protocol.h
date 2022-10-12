#pragma once
#include <string>

enum MessageType
{
	JOIN,
	MESSAGE,
	LEAVE,
};

struct PacketHeader
{
	int packetLength = 0;
	int messageType = 0;
};

struct MessageData
{
	std::string roomName = "";
	std::string message = "";
	std::string senderName = "";
};

struct MessagePacket
{
	PacketHeader header;
	MessageData content;
};
