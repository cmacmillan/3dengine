#pragma once

#include <Windows.h>
#include <string>

struct SFile
{
	SFile();
	~SFile();

	bool FTryRead(const char * pChzPath);
	std::string StrGet();

	FILETIME FiletimeLastWrite();

	unsigned char * m_pB = nullptr;
	size_t m_cBytesFile = -1;
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
};

bool FTryReadFile(const char * pChzPath, SFile * pFile);

std::string StrGetLastError();
