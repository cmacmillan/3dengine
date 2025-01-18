#include "binarystream.h"

#include <string>

char SBinaryStream::CharRead()
{
	return m_pB[m_i++];
}

unsigned char SBinaryStream::UcharRead()
{
	return m_pB[m_i++];
}

short SBinaryStream::ShortRead()
{
	short s = m_pB[m_i] | (m_pB[m_i+1] << 8);
	m_i += 2;
	return s;
}

unsigned short SBinaryStream::UshortRead()
{
	unsigned short s = m_pB[m_i] | (m_pB[m_i+1] << 8);
	m_i += 2;
	return s;
}

int SBinaryStream::IntRead()
{
	int n = m_pB[m_i] | (m_pB[m_i+1] << 8) | (m_pB[m_i+2] << 16) | (m_pB[m_i+3] << 24);
	m_i += 4;
	return n;
}

unsigned int SBinaryStream::UintRead()
{
	unsigned int n = m_pB[m_i] | (m_pB[m_i+1] << 8) | (m_pB[m_i+2] << 16) | (m_pB[m_i+3] << 24);
	m_i += 4;
	return n;
}

const char * SBinaryStream::PChzRead()
{
	const char * pChz = (const char *)m_pB + m_i;
	int c = strlen(pChz);
	m_i = m_i + c + 1;
	return pChz;
}