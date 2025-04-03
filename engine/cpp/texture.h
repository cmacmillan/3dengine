#pragma once

#include "object.h"
#include <d3d11_1.h>

struct STexture : SObject // texture
{
	typedef SObject super;
	STexture(const char * pChzFilename, bool fIsNormal, bool fGenerateMips, TYPEK typek = TYPEK_Texture);
	STexture(TYPEK typek = TYPEK_Texture);

	SHandle<STexture> HTexture() { return (SHandle<STexture>) m_h; }

	ID3D11Texture2D * m_pD3dtexture = nullptr;
	ID3D11ShaderResourceView * m_pD3dsrview = nullptr;
	ID3D11SamplerState * m_pD3dsamplerstate = nullptr;

	int m_dX;
	int m_dY;
};
