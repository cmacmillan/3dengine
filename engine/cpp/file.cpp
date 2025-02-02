#include "file.h"
#include "object.h"
#include "engine.h"

SFile::SFile()
{
}

SFile::~SFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
	}

	if (m_pB)
	{
		delete m_pB;
	}
}

bool SFile::FTryRead(const char * pChzPath)
{
    m_hFile = CreateFileA(pChzPath,
						   GENERIC_READ,		   // open for reading
						   0,                      // do not share
						   NULL,                   // default security
						   OPEN_EXISTING,		   // open existing only
						   FILE_ATTRIBUTE_NORMAL,  // normal file
						   NULL);                  // no attr. template

    if (m_hFile == INVALID_HANDLE_VALUE) 
		return false;

	m_cBytesFile = GetFileSize(m_hFile, nullptr);
	m_pB = new unsigned char[m_cBytesFile];

	if (!ReadFile(m_hFile, m_pB, (DWORD) m_cBytesFile, nullptr, nullptr))
	{
		m_pB = nullptr;
		m_cBytesFile = -1;
		return false;
	}

	return true;
}

std::string SFile::StrGet()
{
	return std::string((char *) m_pB, m_cBytesFile);
}

FILETIME SFile::FiletimeLastWrite()
{
	FILETIME filetime;
	VERIFY(GetFileTime(m_hFile, nullptr, nullptr, &filetime));
	return filetime;
}

bool FTryReadFile(const char * pChzPath, SFile * pFile)
{
	// BB would be nice to have a stack allocated string class

	std::string strPath = g_game.m_strAssetPath + "\\" + pChzPath;
	return pFile->FTryRead(strPath.c_str());
}
