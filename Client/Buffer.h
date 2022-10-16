#pragma once

#include <vector>
#include <string>

// Buffer Class
class Buffer {
public:
	Buffer(size_t size);

	void WriteInt32LE(std::size_t index, int32_t value);
	void WriteInt32LE(int32_t value);

	void WriteInt16LE(std::size_t index, int16_t value);
	void WriteInt16LE(int16_t value);

	void WriteString(std::size_t index, std::string value);
	void WriteString(std::string value);

	int32_t ReadInt32LE(std::size_t index);
	int32_t ReadInt32LE();

	int16_t ReadInt16LE(std::size_t index);
	int16_t ReadInt16LE();

	std::string ReadString(std::size_t length);
	std::vector<uint8_t> m_Buffer;


private:
	int m_WriteIndex = 0;
	int m_ReadIndex = 0;
	size_t N;
};