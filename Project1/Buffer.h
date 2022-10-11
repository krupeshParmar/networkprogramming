#pragma once

#include <vector>
#include <string>

class Buffer {
public:
	Buffer(size_t size);

	void WriteInt32LE(std::size_t index, int32_t value);
	void WriteInt32LE(int32_t value);

	void WriteInt16LE(std::size_t index, int16_t value);
	void WriteInt16LE(int16_t value);

	void WriteStrLE(std::size_t index, std::size_t value);
	void WriteStrLE(std::size_t value);

	int32_t ReadInt32LE(std::size_t index);
	int32_t ReadInt32LE();

	int16_t ReadInt16LE(std::size_t index);
	int16_t ReadInt16LE();

	std::string ReadStrLE(std::size_t value);
	std::string ReadStrLE();

private:
	std::vector<uint8_t> m_Buffer;
	int m_WriteIndex = 0;
	int m_ReadIndex = 0;
};