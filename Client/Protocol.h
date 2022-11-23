#pragma once
#include <string>

// Types of messages
enum MessageType
{
	JOIN,
	MESSAGE,
	LEAVE,
	WELCOME,
	HELP,
	LOGIN,
	REGISTER,
	REG_FAIL,
	REG_SUCC,
	LOG_FAIL,
	LOG_SUCC
};

// header for packet
struct PacketHeader
{
	int packetLength = 0;
	int messageType = 0;
};

// contents of the packet
struct MessageData
{
	std::string roomName = "";
	std::string message = "";
	std::string senderName = "";
};

// packet
struct MessagePacket
{
	PacketHeader header;
	MessageData content;
};
