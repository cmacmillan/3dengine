#include "util.h"

float maprange(float a1, float a2, float b1, float b2, float g)
{
	float gLerp = (g - a1) / (a2 - a1);
	return lerp(b1, b2, gLerp);
}

bool FIsUpper(char ch)
{
	return 'A' <= ch && ch <= 'Z';
}

bool FIsLower(char ch)
{
	return 'a' <= ch && ch <= 'a';
}

// https://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar-t

std::wstring wstrFromStr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::string strFromWstr(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

void AuditFixArray()
{
	SFixArray<int, 5> aryN;
	aryN.Append(5);
	aryN.Append(4);
	ASSERT(aryN[0] == 5);
	ASSERT(aryN[1] == 4);
	aryN[1] = 10;
	ASSERT(aryN[1] == 10);
	aryN.Append(6);
	aryN.Remove(0);
	ASSERT(aryN[0] == 10);
	ASSERT(aryN[1] == 6);
	aryN.Append(20);
	aryN.RemoveSwap(0);
	ASSERT(aryN[0] == 20);
}
