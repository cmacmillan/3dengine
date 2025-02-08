#include "font.h"

#include <windows.h>
#include "texture.h"
#include "util.h"
#include "file.h"

struct SFontBlockHeader
{
	SFontBlockHeader(SBinaryStream * pBs);
	char m_bType;
	unsigned int m_cBytes;
};

// https://www.angelcode.com/products/bmfont/doc/file_format.html#bin

SFontBlockHeader::SFontBlockHeader(SBinaryStream * pBs)
{
	m_bType = pBs->CharRead();
	m_cBytes = pBs->UintRead();
}

SFontInfoBlock::SFontInfoBlock(SBinaryStream * pBs)
{
	m_nFontSize = pBs->ShortRead();
	m_bBitField = pBs->CharRead();
	m_bCharSet = pBs->UcharRead();
	m_nStretchH = pBs->UshortRead();
	m_bAA = pBs->UcharRead();
	m_bPaddingUp = pBs->UcharRead();
	m_bPaddingRight = pBs->UcharRead();
	m_bPaddingDown = pBs->UcharRead();
	m_bPaddingLeft = pBs->UcharRead();
	m_bSpacingHoriz = pBs->UcharRead();
	m_bSpacingVert = pBs->UcharRead();
	m_bOutline = pBs->UcharRead();
}

SFontCommonBlock::SFontCommonBlock(SBinaryStream * pBs)
{
	m_nLineHeight = pBs->UshortRead();
	m_nBase = pBs->UshortRead();
	m_nScaleW = pBs->UshortRead();
	m_nScaleH = pBs->UshortRead();
	m_nPages = pBs->UshortRead();
	m_bBitField = pBs->CharRead();
	m_bAlphaChnl = pBs->UcharRead();
	m_bRedChnl = pBs->UcharRead();
	m_bGreenChnl = pBs->UcharRead();
	m_bBlueChnl = pBs->UcharRead();
}

SFontChar::SFontChar(SBinaryStream * pBs)
{
	m_nId = pBs->UintRead();
	m_nX = pBs->UshortRead();
	m_nY = pBs->UshortRead();
	m_nWidth = pBs->UshortRead();
	m_nHeight = pBs->UshortRead();
	m_nXOffset = pBs->ShortRead();
	m_nYOffset = pBs->ShortRead();
	m_nXAdvance = pBs->ShortRead();
	m_nPage = pBs->UcharRead();
	m_nChnl = pBs->UcharRead();
}

SFontKernPair::SFontKernPair(SBinaryStream * pBs)
{
	m_nFirst = pBs->UintRead();
	m_nSecond = pBs->UintRead();
	m_nAmount = pBs->ShortRead();
}

SFont::SFont(const char * pChzBitmapfontFile, TYPEK typek) : super(typek)
{
	SFile file;
	VERIFY(FTryReadFile(pChzBitmapfontFile, &file));

	SBinaryStream bs(file.m_pB);
	ASSERT(bs.CharRead() == 'B' && bs.CharRead() == 'M' && bs.CharRead() == 'F' && bs.CharRead() == 3);

	SFontBlockHeader fontbhInfo = SFontBlockHeader(&bs);
	m_fontib = SFontInfoBlock(&bs);
	const char * pChzFontName = bs.PChzRead();
	m_strFontName = std::string(pChzFontName);

	SFontBlockHeader fontbhCommon = SFontBlockHeader(&bs);
	m_fontcb = SFontCommonBlock(&bs);

	SFontBlockHeader fontbhNames = SFontBlockHeader(&bs);
	for (int i = 0; i < m_fontcb.m_nPages; i++)
	{
		const char * pChzFile = bs.PChzRead();
		m_aryhTexture.push_back((new STexture(StrPrintf("fonts\\%s", pChzFile).c_str(), false, false))->HTexture());
	}

	SFontBlockHeader fontbhChars = SFontBlockHeader(&bs);
	int cFontchar = fontbhChars.m_cBytes / 20;
	for (int i = 0; i < cFontchar; i++)
	{
		m_aryFontchar.push_back(SFontChar(&bs));
	}

	// BB this header & block may be missing if no kern pairs are present

	SFontBlockHeader fontbhKern = SFontBlockHeader(&bs);
	int cFontkernpair = fontbhKern.m_cBytes / 10;
	for (int i = 0; i < cFontkernpair; i++)
	{
		m_aryFontkernpair.push_back(SFontKernPair(&bs));
	}
}
