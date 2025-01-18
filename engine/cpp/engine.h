#pragma once

#define DEBUG_BUILD

// Uses some starter code from https://github.com/kevinmoran/BeginnerDirect3D11

#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include "object.h"
#include "util.h"
#include "slotheap.h"
#include "binarystream.h"
#include "font.h"
#include "material.h"

#include <windows.h>
#include <vector>
#include <string>
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "vector.h"

//#include "stb_image.h"

// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

struct SVertData3D
{
	Point	m_pos;
	float2	m_uv;
};
CASSERT(sizeof(SVertData3D) == 24);

struct SVertData2D
{
	float2	m_pos;
	float2	m_uv;
};
CASSERT(sizeof(SVertData2D) == 16);

void PushQuad2D(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<SVertData2D> * paryVertdata, std::vector<unsigned short> * paryIIndex);
void PushQuad3D(std::vector<float> * pAryVert, std::vector<unsigned short> * paryIIndex);

struct SMesh3D : SObject // mesh
{
	typedef SObject super;
	SMesh3D();
	SMesh3DHandle HMesh() { return (SMesh3DHandle) m_nHandle; }

	std::vector<SVertData3D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};

struct SMesh2D : SObject // mesh
{
	typedef SObject super;
	SMesh2D();
	SMesh2DHandle HMesh() { return (SMesh2DHandle) m_nHandle; }

	std::vector<SVertData2D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};

struct ShaderGlobals
{
	float m_t;
	float2 m_vecWinSize;

	float m_padding;
};

struct SNode : SObject // node
{
	typedef SObject super;
	SNodeHandle HNode() { return (SNodeHandle) m_nHandle; }

	SNode(SNodeHandle hNodeParent);
	void SetParent(SNodeHandle hNodeParent);

	virtual void Update() {}

	SNodeHandle m_hNodeParent = -1;

	SNodeHandle m_hNodeSiblingPrev = -1;
	SNodeHandle m_hNodeSiblingNext = -1;

	SNodeHandle m_hNodeChildFirst = -1;
	SNodeHandle m_hNodeChildLast = -1;

};

struct SNode3D : SNode // node3D
{
	typedef SNode super;
	SNode3DHandle HNode3D() { return (SNode3DHandle) m_nHandle; }

	SNode3D(SNodeHandle hNodeParent);

	Transform m_transformLocal;
};

struct SCamera3D : SNode3D // camera3D
{
	typedef SNode3D super;
	SCamera3DHandle HCamera3D() { return (SCamera3DHandle) m_nHandle; }

	SCamera3D(SNodeHandle hNodeParent, float radFovHorizontal, float xNearClip, float xFarClip);

	float m_radFovHorizontal = -1;
	float m_xNearClip = -1;
	float m_xFarClip = -1;
};

struct SDrawNodeRenderConstants
{
	Mat m_matMVP;
};

struct SDrawNode3D : SNode3D // drawnode3D
{
	typedef SNode3D super;
	SDrawNode3DHandle HDrawnode3D() { return (SDrawNode3DHandle) m_nHandle; }

	SDrawNode3D(SNodeHandle hNodeParent);

	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};

struct SUiNodeRenderConstants
{
	float2 m_posCenter;
	float2 m_vecScale;
	float4 m_color;
};

struct SUiNode : SNode // uinode
{
	typedef SNode super;
	SUiNodeHandle HUinode() { return (SUiNodeHandle) m_nHandle; }

	SUiNode(SNodeHandle hNodeParent);
	void GetRenderConstants(SUiNodeRenderConstants * pUinoderc);

	float2 m_pos = { 0.0f, 0.0f };
	float2 m_vecScale = { 1.0f, 1.0f };

	float4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_gSort = 0.0f; // Lower = drawn first
	SMaterialHandle m_hMaterial = -1;
	SMesh2DHandle m_hMesh = -1;
};

struct SText : SUiNode // text
{
	typedef SUiNode super;
	STextHandle HText() { return (STextHandle) m_nHandle; }

	SText(SFontHandle hFont, SNodeHandle hNodeParent);
	~SText();

	void SetText(const std::string & str);

	SFontHandle m_hFont = -1;
	std::string m_str;
};

struct SGame // game 
{
	SGame();

	void Init(HINSTANCE hInstance);
	void MainLoop();
	LRESULT LresultWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	float2 VecWinSize();
	void VkPressed(int vk);
	void VkReleased(int vk);

	SNodeHandle	m_hNodeRoot = -1;

	// Gameplay

	SMaterialHandle m_hMaterialTile;
	STextHandle m_hText;

	SCamera3DHandle m_hCamera3D;
	SDrawNode3DHandle m_hPlaneTest;

	// Fonts

	SFontHandle m_hFont;

	SMaterialHandle m_hMaterialText;

	// Window

	HWND m_hwnd;
	bool m_fDidWindowResize = false;

	// Input

	int m_xCursor = 0;
	int m_yCursor = 0;
	bool m_mpVkFDown[0xFF];
	bool m_mpVkFJustPressed[0xFF];
	bool m_mpVkFJustReleased[0xFF];
	float m_sScroll = 0.0f;

	// D3D

	SMesh3DHandle m_hMeshQuad = -1;

	ID3D11Device1 * m_pD3ddevice = nullptr;
	ID3D11DeviceContext1 * m_pD3ddevicecontext = nullptr;
	IDXGISwapChain1 * m_pD3dswapchain = nullptr;
	ID3D11RenderTargetView * m_pD3dframebufferview = nullptr;
	ID3D11DepthStencilView * m_pD3ddepthstencilview = nullptr;

	ID3D11BlendState1 * m_pD3dblendstatenoblend = nullptr;
	ID3D11RasterizerState * m_pD3drasterizerstate = nullptr;
	ID3D11DepthStencilState * m_pD3ddepthstencilstate = nullptr;

	ID3D11Buffer * m_cbufferVertex3D = nullptr;
	ID3D11Buffer * m_cbufferVertex2D = nullptr;
	ID3D11Buffer * m_cbufferIndex = nullptr;
	ID3D11Buffer * m_cbufferUiNode = nullptr;
	ID3D11Buffer * m_cbufferDrawnode3D = nullptr;
	ID3D11Buffer * m_cbufferGlobals = nullptr;

	// Timing

	float m_dT;
	float m_dTSyst;

	double m_dTSystDouble = 0.0;
	LONGLONG m_startPerfCount = 0;
	LONGLONG m_perfCounterFrequency = 0;
};

extern SGame g_game;