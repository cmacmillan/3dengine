#pragma once

#include "object.h"
#include <d3d11_1.h>
#include "linearmap.h"

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

// Hot-reloadable things

struct SShaderData
{
	SHADERK							m_shaderk = SHADERK_Nil;
	std::vector<SNamedTextureSlot>	m_mpISlotStrName = {};
	bool							m_fShadowcast = true;

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
	SShader(const char * pChzFile, SKv<std::string, std::string> * aKvReplacement = nullptr, int cKvReplacement = -1, TYPEK typek = TYPEK_Shader);
	~SShader();

	static bool FTryLoadFromFile(SFile * pFile, SKv<std::string, std::string> * aKvReplacement, int cKvReplacement, SShaderData * pData, std::string * pStrError);
	SShaderHandle HShader() { return (SShaderHandle) m_h; }
	int CNamedslot() const { return m_data.m_mpISlotStrName.size(); }
	void UpdateHotload();
	void ReleaseResources();

	std::string							m_strFile;
	FILETIME							m_filetimeLastEdit = {};

	SShaderData							m_data;
	SKv<std::string, std::string> *		m_aKvReplacement = nullptr; // NOTE this should point towards static consts (not something that might go away)
	int									m_cKvReplacement = -1;
};
