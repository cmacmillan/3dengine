#include "util.h"

#include <cstdarg>

float GMapRange(float a1, float a2, float b1, float b2, float g)
{
	float gLerp = (g - a1) / (a2 - a1);
	return Lerp(b1, b2, gLerp);
}

float GSin(float g)
{
	return float(std::sin(g));
}

float GCos(float g)
{
	return float(std::cos(g));
}

float GTan(float g)
{
	return float(std::tan(g));
}

float GAbs(float g)
{
	if (g < 0.0f)
		return -g;

	return g;
}

float GSqrt(float g)
{
	return sqrtf(g);
}

float GPow(float gBase, float gExponent)
{
	return float(std::pow(gBase, gExponent));
}

float RadFromDeg(float deg)
{
	return deg * PI / 180.0f;
}

float DegFromRad(float rad)
{
	return rad * 180.0f / PI;
}

bool FIsNear(float a, float b, float gEpsilon)
{
	if (GAbs(a - b) < gEpsilon)
		return true;
	return false;
}

bool FIsNear(float a, float b)
{
	const float s_gEpsilon = .00001f;
	return FIsNear(a, b, s_gEpsilon);
}

float GRound(float g, int nDecimalPlaces)
{
	float gScalar = GPow(g, nDecimalPlaces);
	return std::round(g * gScalar) / gScalar;
}

void AuditGRound()
{
	ASSERT(FIsNear(GRound(1.567, 0), 2.0f));
	ASSERT(FIsNear(GRound(1.567, 1), 1.6f));
	ASSERT(FIsNear(GRound(1.567, 2), 1.57));
	ASSERT(FIsNear(GRound(1.567, 3), 1.567));
}

int NFloor(float g)
{
	return int(std::floor(g));
}

int NCeil(float g)
{
	return int(std::ceil(g));
}

bool FIsUpper(char ch)
{
	return 'A' <= ch && ch <= 'Z';
}

bool FIsLower(char ch)
{
	return 'a' <= ch && ch <= 'a';
}

bool FIsWhitespace(char ch)
{
	return ch == ' ' || ch == '\t';
}

bool FMatchCaseInsensitive(const std::string & str1, const std::string & str2)
{
	if (str1.size() != str2.size())
		return false;

	int cCh = str1.size();
	for (int iCh = 0; iCh < cCh; iCh++)
		if (ChToLower(str1[iCh]) != ChToLower(str2[iCh]))
			return false;

	return true;
}

// https://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar-t

std::wstring WstrFromStr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::string StrFromWstr(std::wstring wstr)
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
