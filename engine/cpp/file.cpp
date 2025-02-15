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

// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
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
	{
		std::string str = GetLastErrorAsString();
		return false;
	}

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
