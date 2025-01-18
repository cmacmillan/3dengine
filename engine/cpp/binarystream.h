#pragma once

struct SBinaryStream
{
	SBinaryStream(unsigned char * pB) : m_pB(pB), m_i(0) {}
	char CharRead();
	unsigned char UcharRead();
	short ShortRead();
	unsigned short UshortRead();
	int IntRead();
	unsigned int UintRead();
	const char * PChzRead();
	unsigned char * m_pB;
	int m_i;
};
