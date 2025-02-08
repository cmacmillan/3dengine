#include "text.h"
#include "mesh.h"
#include "font.h"
#include "texture.h"

SText::SText(SFontHandle hFont, SNodeHandle hNodeParent, const std::string & str, TYPEK typek) : super(hNodeParent, str, typek)
{
	m_hMesh = (new SMesh3D())->HMesh();
	m_hFont = hFont;
}

SText::~SText()
{
	delete m_hMesh.PT();
}

void SText::SetText(const std::string & str)
{
	SFont * pFont = m_hFont.PT();

	ASSERT(pFont->m_aryhTexture.size() == 1);
	STexture * pTexture = pFont->m_aryhTexture[0].PT();

	SFontKernPair * pFontkernpair = nullptr;
	char charPrev = '\0';

	m_hMesh->m_aryVertdata.clear();
	m_hMesh->m_aryIIndex.clear();
	std::vector<SVertData3D> & aryVertdata = m_hMesh->m_aryVertdata;
	std::vector<unsigned short> & aryIIndex = m_hMesh->m_aryIIndex;
	
	float x = 0;
	float y = 0;
	for (char charCur : str)
	{
		if (charCur == '\n')
		{
			x = 0;
			y -= pFont->m_fontcb.m_nLineHeight;
			continue;
		}

		const SFontChar * pFontchar = nullptr;
		for (int i = 0; i < pFont->m_aryFontchar.size(); i++)
		{
			const SFontChar & fontchar = pFont->m_aryFontchar[i];
			if (fontchar.m_nId == charCur)
			{
				pFontchar = &pFont->m_aryFontchar[i];
				break;
			}
		}

		ASSERT(pFontchar);

		// See https://www.angelcode.com/products/bmfont/doc/render_text.html
		//  The idea here is the "base" line is the 0 coordinate for y

		float2 vecDivisor = float2(pTexture->m_dX, pTexture->m_dY);
		float2 vecExtents = float2(pFontchar->m_nWidth, pFontchar->m_nHeight);
		float2 uvMin = float2(pFontchar->m_nX / float(pTexture->m_dX), pFontchar->m_nY / float(pTexture->m_dY));
		float2 uvMax = uvMin + vecExtents / vecDivisor;
		float2 posMax = float2(x + vecExtents.m_x, y - pFontchar->m_nYOffset); //+ pFont->m_fontcb.m_nBase - pFontchar->m_nYOffset);
		float2 posMin = float2(x, y - pFontchar->m_nYOffset - vecExtents.m_y);
		PushQuad2D(posMin, posMax, uvMin, uvMax, &aryVertdata, &aryIIndex);

		// TODO support character-pair based kerning

		x += pFontchar->m_nXAdvance;
	}
}