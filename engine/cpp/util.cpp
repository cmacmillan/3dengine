#include "util.h"

#include <cstdarg>

float GMapRange(float a1, float a2, float b1, float b2, float g)
{
	float gLerp = (g - a1) / (a2 - a1);
	return Lerp(b1, b2, gLerp);
}

float GSin(float rad)
{
	return float(std::sin(rad));
}

float GCos(float rad)
{
	return float(std::cos(rad));
}

float GTan(float rad)
{
	return float(std::tan(rad));
}

float RadAsin(float g)
{
	return float(std::asin(g));
}

float RadAcos(float g)
{
	return float(std::acos(g));
}

float RadAtan(float g)
{
	return float(std::atan(g));
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

void DoNothing() { }

float GPow(float gBase, float gExponent)
{
	return float(std::pow(gBase, gExponent));
}

int NPow(int nBase, int nExponent)
{
	return pow(nBase, nExponent);
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
	const float s_gEpsilon = .0001f;
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

float GSign(float g)
{
	if (g < 0.0f)
		return -1.0f;
	return 1.0f;
}

int NSign(int n)
{
	if (n < 0)
		return -1;
	return 1;
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


bool FChIsNumber(char ch)
{
	return ch >= '0' && ch <= '9';
}

int NFromCh(char ch)
{
	if (FChIsNumber(ch))
	{
		return ch - '0';
	}
	ASSERT(false);
	return -1;
}

int NFromStr(const std::string & str)
{
	int iStrStart = 0;
	int cStr = str.size();
	int nSign = 1;
	if (FChIsNumber(str[0]))
	{
		// ...
	}
	else if (str[0] == '-')
	{
		nSign = -1;
		iStrStart++;
	}
	else
	{
		ASSERT(false);
		return -1;
	}

	int n = 0;
	for (int iCh = iStrStart; iCh < cStr; iCh++)
	{
		int nPow = (cStr - 1) - iCh;
		n += NFromCh(str[iCh]) * NPow(10, nPow);
	}

	return n * nSign;
}

void AuditNFromStr()
{
	ASSERT(NFromStr(std::string("-15")) == -15);
	ASSERT(NFromStr(std::string("234")) == 234);
	ASSERT(NFromStr(std::string("0")) == 0);
	ASSERT(NFromStr(std::string("1")) == 1);
	ASSERT(NFromStr(std::string("-1")) == -1);
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
