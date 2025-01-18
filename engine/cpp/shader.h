#pragma once

#include "object.h"
#include <d3d11_1.h>

enum SHADERK
{
	SHADERK_Ui = 0,
	SHADERK_3D = 1,

	SHADERK_Nil = -1,
};

struct SShader : SObject // shader
{
	typedef SObject super;
	SShader(const char * pChzFile);
	SShaderHandle HShader() { return (SShaderHandle) m_nHandle; }

	SHADERK m_shaderk = SHADERK_Nil;
	ID3D11VertexShader * m_pD3dvertexshader = nullptr;
	ID3D11PixelShader * m_pD3dfragshader = nullptr;
	ID3D11InputLayout * m_pD3dinputlayout = nullptr;
};
