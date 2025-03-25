#pragma once

#define DEBUG_BUILD

// Uses some starter code from https://github.com/kevinmoran/BeginnerDirect3D11

#define WIN32_LEAN_AND_MEAN
#define UNICODE

#define TESTING_BIG_MAP 0

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
#include "color.h"

#include <windows.h>
#include <vector>
#include <string>
#include <d3d11_1.h>
#include "vector.h"

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

#define DEBUG_RAYCAST 0

struct ShaderGlobals
{
	float m_t;
	float m_dT;
	float2 m_vecWinSize;

	Mat		m_matCameraToWorld;
    Mat		m_matWorldToCamera;
	Mat		m_matClipToWorld;
    Mat		m_matWorldToClip;

    Mat		m_matWorldToShadowClip;

	Vector	m_normalSunDir;

	float	m_xClipNear;
	float	m_xClipFar;
	float	m_radHFov;
	float	m_padding;
};

enum EDITS
{
	EDITS_Editor = 0,
	EDITS_Player = 1,

	EDITS_Nil = -1,
};

enum DDK
{
	DDK_Sphere = 0,
	DDK_Cube = 1,
	DDK_Arrow = 2,
	DDK_Line = 3,
};

enum DDSTYLE
{
	DDSTYLE_Wireframe,
	DDSTYLE_Solid,
	DDSTYLE_Outline
};

struct SDebugDraw
{
	DDK		m_ddk;
	Mat		m_mat;
	Mat		m_mat2;		// Extra mat for arrow
	SRgba	m_rgba;
	double	m_systRealtimeExpire;
	float	m_gSort;	// Lower = draw first
	DDSTYLE m_ddstyle;

	float	m_gSorted;	// Don't use externally
};

// immediate mode gui id

#define IMGUI() (void*)__FUNCTION__
struct SUiid // uiid
{
	bool operator==(const SUiid & uiidOther) const;

	void *	m_pVFunction;
	int		m_id;
	int		m_index;
};

extern SUiid g_uiidNil;

struct SUiidOverlap
{
	SUiid m_uiid;
	float m_s;	// Distance along the cursor ray. (-1 for 2d ui)
};

struct SGame // game 
{
	SGame();

	void Init(HINSTANCE hInstance);
	void MainLoop();
	LRESULT LresultWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	float2 VecWinSize();
	float2 VecCursor();
	float2 VecWinTopLeft();

	Point PosCursorRay(Vector * pNormalResult);

	bool FRaycastCursor(Point * pPosResult);

	void PrintConsole(const std::string & str, float dTRealtime = 0.0f);

	void DebugDrawSphere(Point posSphere, float sRadius = 1.0f, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);
	void DebugDrawCube(const Mat & mat, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);
	void DebugDrawArrow(Point pos, Vector dPos, float sRadius = 0.1f, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);
	void DebugDrawArrow(Point pos0, Point pos1, float sRadius = 0.1f, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);
	void DebugDrawLine(Point pos0, Point pos1, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);
	void DebugDrawLine(Point pos, Vector dPos, float dTRealtime = 0.0f, SRgba rgba = SRgba(0.0f, 1.0f, 0.0f, 1.0f), float gSort = 0.0f, DDSTYLE ddstyle = DDSTYLE_Wireframe);

	Point PosImgui(Point posCur, const SUiid & uiid);
	Point PosSingleArrowImgui(Point posCur, const SUiid & uiid, SRgba rgba, Vector normal, float sLengthArrow);

	void EnsureMeshIn3dCbuffer(
			SMesh3D * pMesh, 
			int * piBIndex, 
			int * piBVert3D, 
			D3D11_MAPPED_SUBRESOURCE * pMappedsubresVerts3D, 
			D3D11_MAPPED_SUBRESOURCE * pMappedsubresIndex);

	void SetEdits(EDITS edits);
	void UpdateEdits();

	float RDT();

	// TODO add QueuePrintConsole for debugging rendering stuff

	// IMGUI

	SUiid m_uiidHot = g_uiidNil;
	SUiid m_uiidActive = g_uiidNil;
	std::vector<SUiidOverlap> m_aryUiidoverlap = {};
	Vector m_dPosActiveLineseg = g_vecZAxis;

	// Misc

	SNodeHandle	m_hNodeRoot = -1;

	SConsoleHandle m_hConsole = -1;

	STextureHandle m_hTextureSkybox = -1;
	SMaterialHandle m_hMaterialSkybox = -1;
	SShaderHandle m_hShaderSkybox = -1;

	SMaterialHandle m_hMaterialShadowcaster = -1;
	SShaderHandle m_hShaderShadowcaster = -1;

	SCamera3DHandle m_hCamera3DMain = -1;
	SCamera3DHandle m_hCamera3DShadow = -1;

	SMaterialHandle m_hMaterialDefault3d = -1;

	SSunHandle m_hSun = -1;

	SPlayerHandle m_hPlayer = -1;
	SFlycamHandle m_hFlycam = -1; // Editor-mode flycam

	EDITS m_edits = EDITS_Nil;

	SMaterialHandle m_hMaterialDebugDrawWireframe = -1;
	SMaterialHandle m_hMaterialDebugDrawSolidNoDepthWrite = -1;
	SMaterialHandle m_hMaterialDebugDrawSolidDepthWrite = -1;
	SMaterialHandle m_hMaterialDebugDrawOutline = -1;

	std::list<SDebugDraw> m_lDdToDraw = {};

#if DEBUG_RAYCAST
	Point m_posRaycastDbg = Point(0.0f, 0.0f, 0.0f);
	Vector m_normalRaycastDbg = Vector(1.0f, 0.0f, 0.0f);
#endif

	// BB remember we have to manually include debug draw meshes in the vertex/index buffers

	SMesh3DHandle m_hMeshSphere = -1;
	SMesh3DHandle m_hMeshCube = -1;
	SMesh3DHandle m_hMeshArrowBody = -1;
	SMesh3DHandle m_hMeshArrowHead = -1;

	// TODO should just have a map of each typek to an array of instantiated objects of that type (would need to include derived classes too)

	//std::vector<SShader *> m_arypShader = {};

	std::string m_strAssetPath = {};

	// Fonts

	SFontHandle m_hFont = -1;
	SMaterialHandle m_hMaterialText = -1;

	// Window

	HWND m_hwnd;
	bool m_fDidWindowResize = false;
	bool m_fWindowFocused = false;

	// Input

	int m_xCursor = 0;
	int m_yCursor = 0;
	bool m_mpVkFDown[0xFF];
	bool m_mpVkFJustPressed[0xFF];
	bool m_mpVkFJustReleased[0xFF];
	float m_sScroll = 0.0f;

	struct SClickHistory // clickhist
	{
		double m_systRealtime;
		float2 m_posCursor;
	};

	SFixArray<SClickHistory, 8> m_aryClickhist = {};

	// D3D

	SMesh3DHandle m_hMeshQuad = -1;

	ID3D11Device1 * m_pD3ddevice = nullptr;
	ID3D11DeviceContext1 * m_pD3ddevicecontext = nullptr;
	IDXGISwapChain1 * m_pD3dswapchain = nullptr;
	ID3D11RenderTargetView * m_pD3dframebufferview = nullptr;
	ID3D11DepthStencilView * m_pD3ddepthstencilview = nullptr;

	ID3D11RenderTargetView * m_pD3dframebufferviewShadow = nullptr;
	ID3D11DepthStencilView * m_pD3ddepthstencilviewShadow = nullptr;
	STextureHandle m_hTextureShadow = -1;

	ID3D11Buffer * m_cbufferVertex3D = nullptr;
	ID3D11Buffer * m_cbufferIndex = nullptr;
	ID3D11Buffer * m_cbufferUiNode = nullptr;
	ID3D11Buffer * m_cbufferDrawnode3D = nullptr;
	ID3D11Buffer * m_cbufferGlobals = nullptr;

	// Timing

	float m_dT = 0.0f;
	float m_dTRealtime = 0.0f;

	double m_syst = 0.0;
	double m_systRealtime = 0.0;

	LONGLONG m_startPerfCount = 0;
	LONGLONG m_perfCounterFrequency = 0;
};

extern SGame g_game;