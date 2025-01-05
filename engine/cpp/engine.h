#pragma once

#define DEBUG_BUILD

#define ASSET_PATH "C:\\Users\\chase\\OneDrive\\Desktop\\3dengine\\engine\\" // TODO make relative instead of absolute

// Uses some starter code from https://github.com/kevinmoran/BeginnerDirect3D11

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include "util.h"
#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "vector.h"
#include "slotheap.h"

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

struct SObject;
struct SObjectManager
{
	void RegisterObj(SObject * pObj);
	void UnregisterObj(SObject * pObj);
	std::unordered_map<int, SObject *> m_mpObjhObj;
	int m_cId = 0;
};
extern SObjectManager g_objman;

template <typename T>
struct SHandle
{
	SHandle() : m_id(-1) {}
	SHandle(int id) : m_id(id) {}
	int m_id = -1;
	T * PT() const
	{
		auto kv = g_objman.m_mpObjhObj.find(m_id);
		if (kv == g_objman.m_mpObjhObj.end()) return nullptr;
		return (T *) kv->second;
	}
	T * operator->() const
	{
		return PT();
	}
	T & operator*() const
	{
		T * pT = PT();
		if (pT == nullptr)
		{
			// Deliberately crash
			*((char *)nullptr) = 0;
		}
		return *pT;
	}
	bool operator==(const SHandle<T> & hOther) const
	{
		return m_id == hOther.m_id;
	}
	bool operator==(const void * pV) const
	{
		return PT() == pV;
	}
	bool operator==(int id) const
	{
		return m_id == id;
	}
};

template <typename T>
bool operator==(const void * pV, const SHandle<T> & hOther)
{
	return pV == hOther.PT();
}

template <typename T>
bool operator==(const int id, const SHandle<T> & hOther)
{
	return id == hOther.m_id;
}

enum TYPEK
{
	// NOTE When adding new elements to this, make sure to add them to TypekSuper too

	TYPEK_Object,
		TYPEK_Texture,
		TYPEK_Shader,
		TYPEK_Material,
		TYPEK_Node,
			TYPEK_UiNode,
				TYPEK_Text,
			TYPEK_Node3D,
				TYPEK_DrawNode3D,
				TYPEK_Camera3D,
		TYPEK_Font,
		TYPEK_Mesh2D,
		TYPEK_Mesh3D,

	TYPEK_Nil = -1,
};

TYPEK TypekSuper(TYPEK typek)
{
	switch (typek)
	{
		case TYPEK_Object:	return TYPEK_Nil;
			case TYPEK_Texture:		return TYPEK_Object;
			case TYPEK_Shader:		return TYPEK_Object;
			case TYPEK_Material:	return TYPEK_Object;
			case TYPEK_Node:		return TYPEK_Object;
				case TYPEK_UiNode:		return TYPEK_Node;
					case TYPEK_Text:		return TYPEK_UiNode;
				case TYPEK_Node3D:		return TYPEK_Node;
					case TYPEK_DrawNode3D:	return TYPEK_Node3D;
					case TYPEK_Camera3D:	return TYPEK_Node3D;
			case TYPEK_Font:		return TYPEK_Object;
			case TYPEK_Mesh2D:		return TYPEK_Object;
			case TYPEK_Mesh3D:		return TYPEK_Object;

			default:
				{
					ASSERT(false);
					return TYPEK_Nil;
				}
	}
}

struct SObject  // obj
{
	SObject();
	~SObject();
	bool FIsDerivedFrom(TYPEK typek);
	TYPEK m_typek = TYPEK_Object;
	int m_nHandle = -1;
};

struct SBinaryStream
{
	SBinaryStream(unsigned char * pB) : m_pB(pB), m_i(0) {}
	char CharRead();
	unsigned char UcharRead();
	short ShortRead();
	unsigned short UshortRead();
	int IntRead();
	unsigned int UintRead();
	const char * PChzRead();
	unsigned char * m_pB;
	int m_i;
};

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
	unsigned short m_nLineHeight;
	unsigned short m_nBase;
	unsigned short m_nScaleW;
	unsigned short m_nScaleH;
	unsigned short m_nPages;
	char m_bBitField;
	unsigned char m_bAlphaChnl;
	unsigned char m_bRedChnl;
	unsigned char m_bGreenChnl;
	unsigned char m_bBlueChnl;
};

struct SFontChar
{
	SFontChar() {}
	SFontChar(SBinaryStream * pBs);
	unsigned int m_nId;
	unsigned short m_nX;
	unsigned short m_nY;
	unsigned short m_nWidth;
	unsigned short m_nHeight;
	short m_nXOffset;
	short m_nYOffset;
	short m_nXAdvance;
	unsigned char m_nPage;
	unsigned char m_nChnl;
};

struct SFontKernPair
{
	SFontKernPair() {}
	SFontKernPair(SBinaryStream * pBs);
	unsigned int m_nFirst;
	unsigned int m_nSecond;
	short m_nAmount;
};

struct STexture : SObject // texture
{
	typedef SObject super;
	STexture(const char * pChzFilename, bool fIsNormal, bool fGenerateMips);
	SHandle<STexture> HTexture() { return (SHandle<STexture>) m_nHandle; }

	ID3D11Texture2D * m_pD3dtexture = nullptr;
	ID3D11ShaderResourceView * m_pD3dsrview = nullptr;
	ID3D11SamplerState * m_pD3dsamplerstate = nullptr;

	int m_dX;
	int m_dY;
};
typedef SHandle<STexture> STextureHandle;

struct SFont : SObject
{
	typedef SObject super;
	SFont(const char * pChzBitmapfontFile);
	SHandle<SFont> HFont() { return (SHandle<SFont>) m_nHandle; }

	SFontInfoBlock m_fontib;
	std::string m_strFontName;
	SFontCommonBlock m_fontcb;
	std::vector<SFontChar> m_aryFontchar;
	std::vector<SFontKernPair> m_aryFontkernpair;

	std::vector<STextureHandle> m_aryhTexture;
};
typedef SHandle<SFont> SFontHandle;

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
	SHandle<SMesh3D> HMesh() { return (SHandle<SMesh3D>) m_nHandle; }

	std::vector<SVertData3D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};
typedef SHandle<SMesh3D> SMesh3DHandle;

struct SMesh2D : SObject // mesh
{
	typedef SObject super;
	SMesh2D();
	SHandle<SMesh2D> HMesh() { return (SHandle<SMesh2D>) m_nHandle; }

	std::vector<SVertData2D>		m_aryVertdata;
	std::vector<unsigned short>		m_aryIIndex;

	// Data used while rendering only
	//  This could potentially be factored out and stored as a pointer or something

	unsigned int					m_iVertdata = -1;
	unsigned int					m_cVerts = -1;
	unsigned int					m_iIndexdata = -1;
	unsigned int					m_cIndicies= -1;
};
typedef SHandle<SMesh2D> SMesh2DHandle;

struct SShader : SObject // shader
{
	typedef SObject super;
	SShader(LPCWSTR lpcwstrFilename, bool fIs3D);
	SHandle<SShader> HShader() { return (SHandle<SShader>) m_nHandle; }

	ID3D11VertexShader * m_pD3dvertexshader = nullptr;
	ID3D11PixelShader * m_pD3dfragshader = nullptr;
	ID3D11InputLayout * m_pD3dinputlayout = nullptr;
};
typedef SHandle<SShader> SShaderHandle;

struct SMaterial : SObject // material
{
	typedef SObject super;
	SMaterial(SShaderHandle hShader);
	SHandle<SMaterial> HMaterial() { return (SHandle<SMaterial>) m_nHandle; }

	STextureHandle m_hTexture = -1;
	STextureHandle m_hTexture2 = -1;
	SShaderHandle m_hShader = -1;
	float2 m_uvTopleft;
};
typedef SHandle<SMaterial> SMaterialHandle;

struct ShaderGlobals
{
	float m_t;
	float2 m_vecWinSize;

	float m_padding;
};

struct SNode : SObject // node
{
	typedef SObject super;
	SHandle<SNode> HNode() { return (SHandle<SNode>) m_nHandle; }

	SNode(SHandle<SNode> hNodeParent);
	void SetParent(SHandle<SNode> hNodeParent);

	virtual void Update() {}

	SHandle<SNode> m_hNodeParent = -1;

	SHandle<SNode> m_hNodeSiblingPrev = -1;
	SHandle<SNode> m_hNodeSiblingNext = -1;

	SHandle<SNode> m_hNodeChildFirst = -1;
	SHandle<SNode> m_hNodeChildLast = -1;

};
typedef SHandle<SNode> SNodeHandle; 

struct SNode3D : SNode // node3D
{
	typedef SNode super;
	SHandle<SNode3D> HNode3D() { return (SHandle<SNode3D>) m_nHandle; }

	SNode3D(SHandle<SNode> hNodeParent);

	Transform m_transformLocal;
};
typedef SHandle<SNode3D> SNode3DHandle;

struct SCamera3D : SNode3D // camera3D
{
	typedef SNode3D super;
	SHandle<SCamera3D> HCamera3D() { return (SHandle<SCamera3D>) m_nHandle; }

	SCamera3D(SHandle<SNode> hNodeParent, float radFovHorizontal, float xNearClip, float xFarClip);

	float m_radFovHorizontal = -1;
	float m_xNearClip = -1;
	float m_xFarClip = -1;
};
typedef SHandle<SCamera3D> SCamera3DHandle;

struct SDrawNodeRenderConstants
{
	Mat m_matMVP;
};

struct SDrawNode3D : SNode3D // drawnode3D
{
	typedef SNode3D super;
	SHandle<SDrawNode3D> HDrawnode3D() { return (SHandle<SDrawNode3D>) m_nHandle; }

	SDrawNode3D(SHandle<SNode> hNodeParent);

	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};
typedef SHandle<SDrawNode3D> SDrawNode3DHandle;

struct SUiNodeRenderConstants
{
	float2 m_posCenter;
	float2 m_vecScale;
	float4 m_color;
};

struct SUiNode : SNode // uinode
{
	typedef SNode super;
	SHandle<SUiNode> HUinode() { return (SHandle<SUiNode>) m_nHandle; }

	SUiNode(SNodeHandle hNodeParent);
	void GetRenderConstants(SUiNodeRenderConstants * pUinoderc);

	float2 m_pos = { 0.0f, 0.0f };
	float2 m_vecScale = { 1.0f, 1.0f };

	float4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_gSort = 0.0f; // Lower = drawn first
	SMaterialHandle m_hMaterial = -1;
	SMesh2DHandle m_hMesh = -1;
};
typedef SHandle<SUiNode> SUiNodeHandle;

struct SText : SUiNode // text
{
	typedef SUiNode super;
	SHandle<SText> HText() { return (SHandle<SText>) m_nHandle; }

	SText(SFontHandle hFont, SNodeHandle hNodeParent);
	~SText();

	void Update() override;

	void SetText(const std::string & str);

	SFontHandle m_hFont = -1;
	std::string m_str;
};
typedef SHandle<SText> STextHandle;

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
