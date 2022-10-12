#include <iostream>
#include "Buffer.h"
#include "Protocol.h"

int main()
{
	MessagePacket packet;

	packet.header.messageType = 1;
	packet.content.roomName = "off-topic";
	packet.content.senderName = "Krupesh";
	packet.content.message = "hello there fellas!!";
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

	for (int i = 0; i < packet.header.packetLength; i++)
	{
		printf("%c\n", buffer.m_Buffer[i]);
	}

	int packetlength = buffer.ReadInt32LE();
	int messageType = buffer.ReadInt16LE();
	int senderNameSize = buffer.ReadInt32LE();
	std::string senderName = buffer.ReadString(senderNameSize);
	int roomNameSize = buffer.ReadInt32LE();
	std::string roomName = buffer.ReadString(roomNameSize);
	int msgSize = buffer.ReadInt32LE();
	std::string msg = buffer.ReadString(msgSize);

	std::cout << "Packet Length: " << packetlength << std::endl;
	std::cout << "Message type: " << messageType << std::endl;
	std::cout << "Sender name size: " << senderNameSize << std::endl;
	std::cout << "Sender name: " << senderName << std::endl;
	std::cout << "Room name size: " << roomNameSize << std::endl;
	std::cout << "Room name: " << roomName << std::endl;
	std::cout << "Msg size: " << msgSize << std::endl;
	std::cout << "Msg: " << msg << std::endl;

	return 0;
}