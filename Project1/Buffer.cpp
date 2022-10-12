#include "Buffer.h"

Buffer::Buffer(size_t size)
{
	m_Buffer.resize(size);
	m_WriteIndex = 0;
	m_ReadIndex = 0;
}

//Buffer::Buffer(uint8_t* buf, size_t size)
//{
//	for (size_t i = 0; i < size; i++)
//	{
//		m_Buffer.push_back(buf[i]);
//	}
//}


void Buffer::WriteInt32LE(std::size_t index, int32_t value)
{
	m_Buffer[index] = value;
	m_Buffer[index + 1] = value >> 8;
	m_Buffer[index + 2] = value >> 16;
	m_Buffer[index + 3] = value >> 24;
	m_WriteIndex = index;
}

void Buffer::WriteInt32LE(int32_t value)
{
	m_Buffer[m_WriteIndex++] = value;
	m_Buffer[m_WriteIndex++] = value >> 8;
	m_Buffer[m_WriteIndex++] = value >> 16;
	m_Buffer[m_WriteIndex++] = value >> 24;
}

void Buffer::WriteInt16LE(std::size_t index, int16_t value)
{
	m_Buffer[index] = value;
	m_Buffer[index + 1] = value >> 8;
	m_Buffer[index + 2] = value >> 16;
	m_Buffer[index + 3] = value >> 24;
	m_WriteIndex = index;
}

void Buffer::WriteInt16LE(int16_t value)
{
	m_Buffer[m_WriteIndex++] = value;
	m_Buffer[m_WriteIndex++] = value >> 8;
	m_Buffer[m_WriteIndex++] = value >> 16;
	m_Buffer[m_WriteIndex++] = value >> 24;
}

int32_t Buffer::ReadInt32LE(size_t index)
{
	int32_t value = m_Buffer[index];
	value |= m_Buffer[index + 1] << 8;
	value |= m_Buffer[index + 2] << 16;
	value |= m_Buffer[index + 3] << 24;
	return value;
}


int32_t Buffer::ReadInt32LE()
{
	int32_t value = m_Buffer[m_ReadIndex++];
	value |= m_Buffer[m_ReadIndex++] << 8;
	value |= m_Buffer[m_ReadIndex++] << 16;
	value |= m_Buffer[m_ReadIndex++] << 24;
	return value;
}

int16_t Buffer::ReadInt16LE(size_t index)
{
	int16_t value = m_Buffer[index];
	value |= m_Buffer[index + 1] << 8;
	value |= m_Buffer[index + 2] << 16;
	value |= m_Buffer[index + 3] << 24;
	return value;
}


int16_t Buffer::ReadInt16LE()
{
	int16_t value = m_Buffer[m_ReadIndex++];
	value |= m_Buffer[m_ReadIndex++] << 8;
	value |= m_Buffer[m_ReadIndex++] << 16;
	value |= m_Buffer[m_ReadIndex++] << 24;
	return value;
}

void Buffer::WriteString(std::size_t index, std::string value)
{
	for (int i = 0; i < value.size(); i++)
	{
		m_Buffer[index + i] = value[i];
	}
}

void Buffer::WriteString(std::string value)
{
	for (int i = 0; i < value.size(); i++)
	{
		m_Buffer[m_WriteIndex++] = value[i];
	}
}

std::string Buffer::ReadString(std::size_t length)
{
	std::string msg = "";
	for (int i = 0; i < length; i++)
	{
		msg += m_Buffer[m_ReadIndex++];
	}
	return msg;
}