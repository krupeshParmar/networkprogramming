#include <string>

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
	PacketHeader packetHeader;
	MessageData content;
};