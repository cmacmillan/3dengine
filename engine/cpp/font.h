#pragma once

#include <string>
#include <vector>

#include "object.h"
#include "binarystream.h"

// https://www.angelcode.com/products/bmfont/doc/file_format.html

struct SFontInfoBlock
{
	SFontInfoBlock() {}
	SFontInfoBlock(SBinaryStream * pBs);
	short m_nFontSize;
	char m_bBitField;
	unsigned char m_bCharSet;
	unsigned short m_nStretchH;
	unsigned char m_bAA;
	unsigned char m_bPaddingUp;
	unsigned char m_bPaddingRight;
	unsigned char m_bPaddingDown;
	unsigned char m_bPaddingLeft;
	unsigned char m_bSpacingHoriz;
	unsigned char m_bSpacingVert;
	unsigned char m_bOutline;
};

struct SFontCommonBlock
{
	SFontCommonBlock() {}
	SFontCommonBlock(SBinaryStream * pBs);
	unsigned short m_nLineHeight;	// This is the distance in pixels between each line of text.
	unsigned short m_nBase;			// The number of pixels from the absolute top of the line to the base of the characters.
	unsigned short m_nScaleW;		// The width of the texture, normally used to scale the x pos of the character image.
	unsigned short m_nScaleH;		// The height of the texture, normally used to scale the y pos of the character image.
	unsigned short m_nPages;		// The number of texture pages included in the font.
	char m_bBitField;				// Set to 1 if the monochrome characters have been packed into each of the texture channels. 
									//  In this case alphaChnl describes what is stored in each channel.

	////////////////////////////////// The following applies to all of the rgba channels:
	unsigned char m_bAlphaChnl;		//  Set to 0 if the channel holds the glyph data, 
	unsigned char m_bRedChnl;		//  1 if it holds the outline, 
	unsigned char m_bGreenChnl;		//  2 if it holds the glyph and the outline, 
	unsigned char m_bBlueChnl;		//  3 if its set to zero, 
									//  and 4 if its set to one.
};

struct SFontChar
{
	SFontChar() {}
	SFontChar(SBinaryStream * pBs);
	unsigned int m_nId;				// The character id.
	unsigned short m_nX;			// The left position of the character image in the texture.
	unsigned short m_nY;			// The top position of the character image in the texture.
	unsigned short m_nWidth;		// The width of the character image in the texture.
	unsigned short m_nHeight;		// The height of the character image in the texture.
	short m_nXOffset;				// How much the current position should be offset when copying the image from the texture to the screen.
	short m_nYOffset;				//  ...
	short m_nXAdvance;				// How much the current position should be advanced after drawing the character.
	unsigned char m_nPage;			// The texture page where the character image is found.
	unsigned char m_nChnl;			// The texture channel where the character image is found 
									//  1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels
};

struct SFontKernPair
{
	SFontKernPair() {}
	SFontKernPair(SBinaryStream * pBs);
	unsigned int m_nFirst;			// The first character id.
	unsigned int m_nSecond;			// The second character id.
	short m_nAmount;				// How much the x position should be adjusted when drawing the second character immediately following the first.
};

struct SFont : SObject
{
	typedef SObject super;
	SFont(const char * pChzBitmapfontFile, TYPEK typek = TYPEK_Font);
	SHandle<SFont> HFont() { return (SHandle<SFont>) m_h; }

	SFontInfoBlock m_fontib;
	std::string m_strFontName;
	SFontCommonBlock m_fontcb;
	std::vector<SFontChar> m_aryFontchar;
	std::vector<SFontKernPair> m_aryFontkernpair;

	std::vector<STextureHandle> m_aryhTexture;
};
