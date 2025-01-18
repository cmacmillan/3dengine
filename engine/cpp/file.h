#pragma once

#include <Windows.h>
#include <string>

struct SFile
{
	SFile();
	~SFile();

	bool FTryRead(const char * pChzPath);
	std::string StrGet();

	unsigned char * m_pB = nullptr;
	size_t m_cBytesFile = -1;
};

bool FTryReadFile(const char * pChzPath, SFile * pFile);
