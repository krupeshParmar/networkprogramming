#include "Buffer.h"
/*
*
*	Student name:	Krupesh Ashok Parmar
*	Student number:	1124374
*	Student mail:	k_parmar180076@fanshaweonline.ca
*
*	Program name:	GAME DEVELOPMENT ADVANCED PROGRAMMING
*	Course name:	INFO6016 - Network Programming
*
*	PROJECT - 1
*	TCP Client & Server, Chat Program
* 
*/
// had linked this project with the other two, but started to get some linker error
// so after panicking a little I just copied all the files to other projects
// and used this project to test buffer and protocol

Buffer::Buffer(size_t size)
{
	m_Buffer.resize(size);
	m_WriteIndex = 0;
	m_ReadIndex = 0;
}

// serialize 4 bytes value
void Buffer::WriteInt32LE(std::size_t index, int32_t value)
{
	m_Buffer[index] = value;
	m_Buffer[index + 1] = value >> 8;
	m_Buffer[index + 2] = value >> 16;
	m_Buffer[index + 3] = value >> 24;
	m_WriteIndex = index;
}

// serialize 4 bytes value
void Buffer::WriteInt32LE(int32_t value)
{
	m_Buffer[m_WriteIndex++] = value;
	m_Buffer[m_WriteIndex++] = value >> 8;
	m_Buffer[m_WriteIndex++] = value >> 16;
	m_Buffer[m_WriteIndex++] = value >> 24;
}

// serialize 2 bytes value
void Buffer::WriteInt16LE(std::size_t index, int16_t value)
{
	m_Buffer[index] = value;
	m_Buffer[index + 1] = value >> 8;
	m_WriteIndex = index;
}

// serialize 4 bytes value
void Buffer::WriteInt16LE(int16_t value)
{
	m_Buffer[m_WriteIndex++] = value;
	m_Buffer[m_WriteIndex++] = value >> 8;
}

// deserialize 4 bytes value
int32_t Buffer::ReadInt32LE(size_t index)
{
	int32_t value = m_Buffer[index];
	value |= m_Buffer[index + 1] << 8;
	value |= m_Buffer[index + 2] << 16;
	value |= m_Buffer[index + 3] << 24;
	return value;
}

// deserialize 4 bytes value
int32_t Buffer::ReadInt32LE()
{
	int32_t value = m_Buffer[m_ReadIndex++];
	value |= m_Buffer[m_ReadIndex++] << 8;
	value |= m_Buffer[m_ReadIndex++] << 16;
	value |= m_Buffer[m_ReadIndex++] << 24;
	return value;
}

// dserialize 2 bytes value
int16_t Buffer::ReadInt16LE(size_t index)
{
	int16_t value = m_Buffer[index];
	value |= m_Buffer[index + 1] << 8;
	return value;
}

// dserialize 2 bytes value
int16_t Buffer::ReadInt16LE()
{
	int16_t value = m_Buffer[m_ReadIndex++];
	value |= m_Buffer[m_ReadIndex++] << 8;
	return value;
}

// serialize string
void Buffer::WriteString(std::size_t index, std::string value)
{
	for (int i = 0; i < value.size(); i++)
	{
		m_Buffer[index + i] = value[i];
	}
}

// serialize string
void Buffer::WriteString(std::string value)
{
	for (int i = 0; i < value.size(); i++)
	{
		m_Buffer[m_WriteIndex++] = value[i];
	}
}

// deserialize string
std::string Buffer::ReadString(std::size_t length)
{
	std::string msg = "";
	for (int i = 0; i < length; i++)
	{
		msg += m_Buffer[m_ReadIndex++];
	}
	return msg;
}