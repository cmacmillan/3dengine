#pragma once

#include "object.h"
#include <d3d11_1.h>

struct SFile;

enum SHADERK
{
	SHADERK_Ui = 0,
	SHADERK_3D = 1,
	SHADERK_Skybox = 2,
	SHADERK_Error = 3,

	SHADERK_Nil = -1,
};

struct SNamedTextureSlot
{
	std::string		m_strName;
	int				m_iSlot;
};

struct SShaderData
{
	D3D11_DEPTH_STENCIL_DESC		m_d3ddepthstencildesc = {};
	D3D11_RASTERIZER_DESC			m_d3drasterizerdesc = {};
	D3D11_RENDER_TARGET_BLEND_DESC1	m_d3drtblenddesc = {};

	ID3D11VertexShader *			m_pD3dvertexshader = nullptr;
	ID3D11PixelShader *				m_pD3dfragshader = nullptr;
	ID3D11InputLayout *				m_pD3dinputlayout = nullptr;
	ID3D11BlendState1 *				m_pD3dblendstatenoblend = nullptr;
	ID3D11RasterizerState *			m_pD3drasterizerstate = nullptr;
	ID3D11DepthStencilState *		m_pD3ddepthstencilstate = nullptr;
};

struct SShader : SObject // shader
{
	typedef SObject super;
	SShader(const char * pChzFile, TYPEK typek = TYPEK_Shader);
	~SShader();

	bool FTryLoadFromFile(SFile * pFile, SShaderData * pData, std::string * pStrError);
	SShaderHandle HShader() { return (SShaderHandle) m_nHandle; }
	int CNamedslot() const { return m_mpISlotStrName.size(); }
	void UpdateHotload();
	void ReleaseResources();

	SHADERK							m_shaderk = SHADERK_Nil;
	std::vector<SNamedTextureSlot>	m_mpISlotStrName = {};

	std::string						m_strFile;
	FILETIME						m_filetimeLastEdit = {};

	SShaderData						m_data;
};
