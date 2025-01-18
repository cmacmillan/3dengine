#pragma once

#include "object.h"
#include <d3d11_1.h>

struct SShader : SObject // shader
{
	typedef SObject super;
	SShader(LPCWSTR lpcwstrFilename, bool fIs3D);
	SShaderHandle HShader() { return (SShaderHandle) m_nHandle; }

	ID3D11VertexShader * m_pD3dvertexshader = nullptr;
	ID3D11PixelShader * m_pD3dfragshader = nullptr;
	ID3D11InputLayout * m_pD3dinputlayout = nullptr;
};
