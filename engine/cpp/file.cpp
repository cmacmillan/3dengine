#include "file.h"
#include "object.h"

SFile::SFile()
{
}

SFile::~SFile()
{
	if (m_pB)
	{
		delete m_pB;
	}
}

bool SFile::FTryRead(const char * pChzPath)
{
    HANDLE hFile = CreateFileA(pChzPath,
							   GENERIC_READ,		   // open for reading
							   0,                      // do not share
							   NULL,                   // default security
							   OPEN_EXISTING,		   // open existing only
							   FILE_ATTRIBUTE_NORMAL,  // normal file
							   NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
		return false;

	m_cBytesFile = GetFileSize(hFile, nullptr);
	m_pB = new unsigned char[m_cBytesFile];

	if (!ReadFile(hFile, m_pB, (DWORD) m_cBytesFile, nullptr, nullptr))
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

bool FTryReadFile(const char * pChzPath, SFile * pFile)
{
	// BB would be nice to have a stack allocated string class

	std::string strPath = std::string(ASSET_PATH) + pChzPath;
	return pFile->FTryRead(strPath.c_str());
}
