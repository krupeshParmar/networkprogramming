#include "Buffer.h"

Buffer::Buffer(size_t size)
{
	m_Buffer.resize(size);
}

void Buffer::WriteInt32LE(size_t index, int32_t value)
{

}
