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
#include "node.h"
#include "node3d.h"
#include "camera3d.h"
#include "drawnode3d.h"
#include "uinode.h"
#include "mesh.h"

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

struct ShaderGlobals
{
	float m_t;
	float2 m_vecWinSize;

	float m_padding;

	Mat		m_matCameraToWorld;
    Mat		m_matWorldToCamera;
	Mat		m_matClipToWorld;
    Mat		m_matWorldToClip;
};

struct SGame // game 
{
	SGame();

	void Init(HINSTANCE hInstance);
	void MainLoop();
	void BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader);
	LRESULT LresultWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	float2 VecWinSize();
	void VkPressed(int vk);
	void VkReleased(int vk);
	void PrintConsole(const std::string & str, float dT = 0.0);

	// Misc

	SNodeHandle	m_hNodeRoot = -1;

	SConsoleHandle m_hConsole = -1;

	STextureHandle m_hTextureSkybox = -1;
	SMaterialHandle m_hMaterialSkybox = -1;
	SShaderHandle m_hShaderSkybox = -1;

	SCamera3DHandle m_hCamera3DMain = -1;
	SDrawNode3DHandle m_hPlaneTest = -1;
	SDrawNode3DHandle m_hPlaneTest2 = -1;

	// Fonts

	SFontHandle m_hFont = -1;
	SMaterialHandle m_hMaterialText = -1;

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

	ID3D11Buffer * m_cbufferVertex3D = nullptr;
	ID3D11Buffer * m_cbufferIndex = nullptr;
	ID3D11Buffer * m_cbufferUiNode = nullptr;
	ID3D11Buffer * m_cbufferDrawnode3D = nullptr;
	ID3D11Buffer * m_cbufferGlobals = nullptr;

	// Timing

	float m_dT = 0.0f;
	double m_dTSyst = 0.0;

	LONGLONG m_startPerfCount = 0;
	LONGLONG m_perfCounterFrequency = 0;
};

extern SGame g_game;