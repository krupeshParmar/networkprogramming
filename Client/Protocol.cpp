#include <string>

struct PacketHeader
{
	int packetLength;
	int messageId;
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