// Uses some d3d api code from https://github.com/kevinmoran/BeginnerDirect3D11

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define nShadowRes 2048

#include "engine.h"
#include "fpscounter.h"
#include "texture.h"
#include "shader.h"
#include "flycam.h"
#include "console.h"
#include "timingcontext.h"
#include "gltfloader.h"
#include "render.h"
#include "sun.h"

#include <exception>

#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

SGame g_game;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return g_game.LresultWindowProcedure(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	g_game = SGame();
	g_game.Init(hInstance);

	g_game.MainLoop();

	return 0;
}

/////////////////

float2 SGame::VecWinSize()
{
	RECT winRect;
	GetClientRect(m_hwnd, &winRect);

	float dxWindow = winRect.right - winRect.left;
	float dyWindow = winRect.bottom - winRect.top;
	return float2(dxWindow, dyWindow);
}

float2 SGame::VecWinTopLeft()
{
	RECT winRect;
	GetWindowRect(m_hwnd, &winRect);

	return float2(winRect.left, winRect.top);
}

SGame::SGame()
{
	for (int i = 0; i < DIM(m_mpVkFDown); i++)
	{
		m_mpVkFDown[i] = false;
		m_mpVkFJustPressed[i] = false;
		m_mpVkFJustReleased[i] = false;
	}
}

int aVkCompute[] = {
	VK_0,
	VK_1,
	VK_2,
	VK_3,
	VK_4,
	VK_5,
	VK_6,
	VK_7,
	VK_8,
	VK_9,
	VK_A,
	VK_B,
	VK_C,
	VK_D,
	VK_E,
	VK_F,
	VK_G,
	VK_H,
	VK_I,
	VK_J,
	VK_K,
	VK_L,
	VK_M,
	VK_N,
	VK_O,
	VK_P,
	VK_Q,
	VK_R,
	VK_S,
	VK_T,
	VK_U,
	VK_V,
	VK_W,
	VK_X,
	VK_Y,
	VK_Z,

	VK_MENU,
	VK_ESCAPE,
	VK_RBUTTON,
	VK_LBUTTON,
	VK_MBUTTON,
	VK_SHIFT,
	VK_CONTROL,
	VK_SPACE,
	VK_BACK,
	VK_TAB,
	VK_RETURN,

	VK_LEFT,
	VK_RIGHT,
	VK_UP,
	VK_DOWN,
};

void SGame::Init(HINSTANCE hInstance)
{
	STimingContext timectx = STimingContext("Init", 15.0f);

	m_hNodeRoot = (new SNode(-1, "RootNode"))->HNode();

	/*
	// https://learn.microsoft.com/en-us/windows/win32/inputdev/using-raw-input
	RAWINPUTDEVICE aRid[] =
	{
		{
		0x01,						// HID_USAGE_PAGE_GENERIC
		0x02,						// HID_USAGE_GENERIC_MOUSE
		0x00,//RIDEV_NOLEGACY,		// adds mouse and also ignores legacy mouse messages
		0,
		}
	};

	VERIFY(RegisterRawInputDevices(aRid, DIM(aRid), sizeof(aRid[0])));
	*/

	// Open a window
	{
		WNDCLASSEXW winClass = {};
		winClass.cbSize = sizeof(WNDCLASSEXW);
		winClass.style = CS_HREDRAW | CS_VREDRAW;
		winClass.lpfnWndProc = &WndProc;
		winClass.hInstance = hInstance;
		winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
		winClass.hCursor = LoadCursorW(0, IDC_ARROW);
		winClass.lpszClassName = L"MyWindowClass";
		winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

		if (!RegisterClassExW(&winClass))
		{
			MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
			exit;
		}

		RECT initialRect = { 0, 0, 1024, 768 };
		AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
		LONG initialWidth = initialRect.right - initialRect.left;
		LONG initialHeight = initialRect.bottom - initialRect.top;

		m_hwnd = CreateWindowExW(
					WS_EX_OVERLAPPEDWINDOW,
					winClass.lpszClassName,
					L"Engine",
					//(WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_SYSMENU,
					WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					CW_USEDEFAULT, 
					CW_USEDEFAULT,
					initialWidth,
					initialHeight,
					0, 
					0, 
					hInstance, 
					0);

		if (!m_hwnd)
		{
			MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
			exit;
		}
	}

	// Get exe path
	{
		WCHAR aChzCurrentDirectory[1000];
		int cChar = GetCurrentDirectory(DIM(aChzCurrentDirectory), aChzCurrentDirectory);
		ASSERT(cChar != 0);

		m_strAssetPath = StrFromWstr(aChzCurrentDirectory);
	}

	// Create D3D11 Device and Context
	{
		ID3D11Device * baseDevice;
		ID3D11DeviceContext * baseDeviceContext;
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG_BUILD)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
			0, creationFlags,
			featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, &baseDevice,
			0, &baseDeviceContext);
		if (FAILED(hResult))
		{
			MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
			exit;
		}

		// Get 1.1 interface of D3D11 Device and Context
		hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void **) &m_pD3ddevice);
		assert(SUCCEEDED(hResult));
		baseDevice->Release();

		hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **) &m_pD3ddevicecontext);
		assert(SUCCEEDED(hResult));
		baseDeviceContext->Release();
	}

#ifdef DEBUG_BUILD
	// Set up debug layer to break on D3D11 errors
	{
		ID3D11Debug * d3dDebug = nullptr;
		m_pD3ddevice->QueryInterface(__uuidof(ID3D11Debug), (void **) &d3dDebug);
		if (d3dDebug)
		{
			ID3D11InfoQueue * d3dInfoQueue = nullptr;
			if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void **) &d3dInfoQueue)))
			{
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
				d3dInfoQueue->Release();
			}
			d3dDebug->Release();
		}
	}
#endif

	// Create Swap Chain
	{
		// Get DXGI Factory (needed to create Swap Chain)
		IDXGIFactory2 * dxgiFactory;
		{
			IDXGIDevice1 * dxgiDevice;
			HRESULT hResult = m_pD3ddevice->QueryInterface(__uuidof(IDXGIDevice1), (void **) &dxgiDevice);
			assert(SUCCEEDED(hResult));

			IDXGIAdapter * dxgiAdapter;
			hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
			assert(SUCCEEDED(hResult));
			dxgiDevice->Release();

			DXGI_ADAPTER_DESC adapterDesc;
			dxgiAdapter->GetDesc(&adapterDesc);

			OutputDebugStringA("Graphics Device: ");
			OutputDebugStringW(adapterDesc.Description);

			hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void **) &dxgiFactory);
			assert(SUCCEEDED(hResult));
			dxgiAdapter->Release();
		}

		DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
		d3d11SwapChainDesc.Width = 0; // use window width
		d3d11SwapChainDesc.Height = 0; // use window height
		d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		d3d11SwapChainDesc.SampleDesc.Count = 1;
		d3d11SwapChainDesc.SampleDesc.Quality = 0;
		d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		d3d11SwapChainDesc.BufferCount = 2;
		d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		d3d11SwapChainDesc.Flags = 0;

		HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(m_pD3ddevice, m_hwnd, &d3d11SwapChainDesc, 0, 0, &m_pD3dswapchain);
		assert(SUCCEEDED(hResult));

		dxgiFactory->Release();
	}

	// Create Framebuffer Render Target

	{
		// BB overlap/duplication with resize code

		ID3D11Texture2D * d3d11FrameBuffer;
		HRESULT hResult = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
		assert(SUCCEEDED(hResult));

		hResult = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, nullptr, &m_pD3dframebufferview);
		assert(SUCCEEDED(hResult));

		// Get the framebuffer from the device

		D3D11_TEXTURE2D_DESC depthBufferDesc;
		d3d11FrameBuffer->GetDesc(&depthBufferDesc);

		d3d11FrameBuffer->Release();

		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D * depthBuffer;
		m_pD3ddevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

		m_pD3ddevice->CreateDepthStencilView(depthBuffer, nullptr, &m_pD3ddepthstencilview);

		depthBuffer->Release(); // BB why do we release here? This seems highly suspect
	}

	// Create Shadow Render Target

	{
		m_hTextureShadow = (new STexture())->HTexture();

		// Create Sampler State

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = 0.0f; // Disable lod
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		g_game.m_pD3ddevice->CreateSamplerState(&samplerDesc, &m_hTextureShadow->m_pD3dsamplerstate);

		// Create Texture
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = nShadowRes;
		textureDesc.Height = nShadowRes;
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		HRESULT hresultTexture = g_game.m_pD3ddevice->CreateTexture2D(&textureDesc, nullptr, &m_hTextureShadow->m_pD3dtexture);
		ASSERT(hresultTexture == S_OK);

		HRESULT hResult = m_pD3ddevice->CreateRenderTargetView(m_hTextureShadow->m_pD3dtexture, nullptr, &m_pD3dframebufferviewShadow);
		assert(SUCCEEDED(hResult));

		D3D11_SHADER_RESOURCE_VIEW_DESC d3dsrvd = {};

		d3dsrvd.Format = textureDesc.Format;
		d3dsrvd.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		d3dsrvd.Texture2D.MipLevels = -1;

		g_game.m_pD3ddevice->CreateShaderResourceView(m_hTextureShadow->m_pD3dtexture, &d3dsrvd, &m_hTextureShadow->m_pD3dsrview);

		D3D11_TEXTURE2D_DESC depthBufferDesc = {};

		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.Width = nShadowRes;
		depthBufferDesc.Height = nShadowRes;
		depthBufferDesc.MipLevels = 0;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		//depthBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		ID3D11Texture2D * depthBuffer;
		m_pD3ddevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

		m_pD3ddevice->CreateDepthStencilView(depthBuffer, nullptr, &m_pD3ddepthstencilviewShadow);

		depthBuffer->Release();
	}

	{
		const int cVertsMax = 100000;

		D3D11_BUFFER_DESC descVertexBuffer = {};
		descVertexBuffer.ByteWidth = cVertsMax * sizeof(SVertData3D);
		descVertexBuffer.Usage = D3D11_USAGE_DYNAMIC;
		descVertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descVertexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descVertexBuffer, nullptr, &m_cbufferVertex3D);
		assert(SUCCEEDED(hResult));
	}

	{
		const int cIndexMax = 100000;

		D3D11_BUFFER_DESC descIndexBuffer = {};
		descIndexBuffer.ByteWidth = cIndexMax * sizeof(unsigned short);
		descIndexBuffer.Usage = D3D11_USAGE_DYNAMIC;
		descIndexBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
		descIndexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descIndexBuffer, nullptr, &m_cbufferIndex);
		assert(SUCCEEDED(hResult));
	}

	{
		D3D11_BUFFER_DESC descCbufferUiNode = {};
		CASSERT(sizeof(SUiNodeRenderConstants) % 16 == 0);
		descCbufferUiNode.ByteWidth = sizeof(SUiNodeRenderConstants);
		descCbufferUiNode.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferUiNode.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferUiNode.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferUiNode, nullptr, &m_cbufferUiNode);
		assert(SUCCEEDED(hResult));
	}

	{
		D3D11_BUFFER_DESC descCbufferDrawnode3D = {};
		CASSERT(sizeof(SDrawNodeRenderConstants) % 16 == 0);
		descCbufferDrawnode3D.ByteWidth = sizeof(SDrawNodeRenderConstants);
		descCbufferDrawnode3D.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferDrawnode3D.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferDrawnode3D.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferDrawnode3D, nullptr, &m_cbufferDrawnode3D);
		assert(SUCCEEDED(hResult));
	}

	{
		D3D11_BUFFER_DESC descCbufferGlobals = {};
		CASSERT(sizeof(ShaderGlobals) % 16 == 0);
		descCbufferGlobals.ByteWidth = sizeof(ShaderGlobals);
		descCbufferGlobals.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferGlobals.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferGlobals.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferGlobals, nullptr, &m_cbufferGlobals);
		assert(SUCCEEDED(hResult));
	}

	// Timing
	{
		LARGE_INTEGER perfCount;
		QueryPerformanceCounter(&perfCount);
		m_startPerfCount = perfCount.QuadPart;
		LARGE_INTEGER perfFreq;
		QueryPerformanceFrequency(&perfFreq);
		m_perfCounterFrequency = perfFreq.QuadPart;
	}

	// Create quad mesh

	{
		m_hMeshQuad = (new SMesh3D())->HMesh();

		PushQuad3D(&m_hMeshQuad->m_aryVertdata, &m_hMeshQuad->m_aryIIndex);
	}

	// Load initial text shader and set up the console

	SShaderHandle hShaderText = (new SShader("shaders\\text2d.hlsl"))->HShader();

	// Font 

	m_hFont = (new SFont("fonts\\candara.fnt"))->HFont();

	m_hMaterialText = (new SMaterial(hShaderText))->HMaterial();
	//m_hMaterialText->m_hTexture = m_hFont->m_aryhTexture[0];
	m_hMaterialText->m_aryNamedtexture.push_back({ m_hFont->m_aryhTexture[0], "fontTexture" });

	// Console

	m_hConsole = (new SConsole(m_hNodeRoot, "Console"))->HConsole();

	// Load other shaders

	SShaderHandle hShader3D = (new SShader("shaders\\unlit3d.hlsl"))->HShader();
	SShaderHandle hShader3DNDotL = (new SShader("shaders\\litndotl.hlsl"))->HShader();

	// Fps counter

	(new SFpsCounter(m_hNodeRoot, "FpsCounter"));

	// Camera

	(new SFlyCam(m_hNodeRoot, "FlyCam"));
	m_hCamera3DShadow = (new SCamera3D(m_hNodeRoot, "AltCam", RadFromDeg(-1.0f), -500.0, 500.0f))->HCamera3D();
	m_hCamera3DShadow->SetOrthographic(200.0f);

	// Skybox

	{
		STimingContext timectx = STimingContext("Loading skybox texture & shader", 15.0f);

		// BB opening this texture is wildly slow (I think it's a big texture problem, not a jpeg problem)
		//m_hTextureSkybox = (new STexture("textures/pretoria_gardens.jpg", false, false))->HTexture();

		m_hTextureSkybox = (new STexture("textures/pretoria_gardens_small.png", false, false))->HTexture();
		m_hShaderSkybox = (new SShader("shaders/skybox.hlsl"))->HShader();
		m_hMaterialSkybox = (new SMaterial(m_hShaderSkybox))->HMaterial();
		m_hMaterialSkybox->m_aryNamedtexture.push_back({ m_hTextureSkybox, "skyTexture" });
	}

	// Shadowcaster

	m_hShaderShadowcaster = (new SShader("shaders/shadowcaster.hlsl"))->HShader();
	m_hMaterialShadowcaster = (new SMaterial(m_hShaderShadowcaster))->HMaterial();

	// Default 3D material

	SMaterial * pMaterial3dNDotL = new SMaterial(hShader3DNDotL);
	g_game.m_hMaterialDefault3d = pMaterial3dNDotL->HMaterial();

	SMaterial * pMaterial3d = new SMaterial(hShader3D);
	pMaterial3d->m_aryNamedtexture.push_back({ (new STexture("textures/testTexture1.png", false, false))->HTexture(),  "mainTexture"});
	pMaterial3d->m_aryNamedtexture.push_back({ (new STexture("textures/testTexture2.png", false, false))->HTexture(),  "altTexture"});

	m_hPlaneTest = (new SDrawNode3D(m_hNodeRoot, "PlaneTest1"))->HDrawnode3D();
	m_hPlaneTest->m_hMaterial = pMaterial3dNDotL->HMaterial();
	m_hPlaneTest->SetPosWorld(Point(10.0f, 0.0f, 0.0f));
	m_hPlaneTest->m_hMesh = m_hMeshQuad;

	m_hPlaneTest2 = (new SDrawNode3D(m_hPlaneTest->HNode(), "PlaneTest2"))->HDrawnode3D();
	m_hPlaneTest2->m_hMaterial = pMaterial3d->HMaterial();
	m_hPlaneTest2->SetPosWorld(Point(10.0f, 2.0f, 0.0f));
	m_hPlaneTest2->m_hMesh = m_hMeshQuad;

	SMesh3D * pMeshSuzzane = PMeshLoadSingle("models/suzanne.gltf");
	SDrawNode3DHandle hDrawnode3dSuzanne = (new SDrawNode3D(m_hPlaneTest->HNode(), "PlaneTest2"))->HDrawnode3D();
	hDrawnode3dSuzanne->m_hMaterial = pMaterial3dNDotL->HMaterial();
	hDrawnode3dSuzanne->m_hMaterial->m_aryNamedtexture.push_back({ (new STexture("textures/uvchecker.jpg",false,false))->HTexture(), "mainTexture" });
	hDrawnode3dSuzanne->m_hMaterial->m_aryNamedtexture.push_back({ m_hTextureShadow, "sunShadowTexture" });
	hDrawnode3dSuzanne->m_hMesh = pMeshSuzzane->HMesh();
	hDrawnode3dSuzanne->SetPosWorld(Point(10.0f, -2.0f, 0.0f));

	SpawnScene("models/fakelevel.gltf");

	// Run audits

	AuditFixArray();
	AuditVectors();
	AuditSlotheap();
	AuditNFromStr();
}

int SortUinodeRenderOrder(const void * pVa, const void * pVb)
{
	SUiNode * pUinodeA = *(SUiNode **) pVa;
	SUiNode * pUinodeB = *(SUiNode **) pVb;
	if (pUinodeA->m_gSort == pUinodeB->m_gSort)
		return 0;
	else if (pUinodeA->m_gSort < pUinodeB->m_gSort)
		return -1;
	else
		return 1;
}

void SGame::MainLoop()
{
	bool isRunning = true;
	while (isRunning)
	{
		{
			double m_dTSystPrev = m_dTSyst;
			LARGE_INTEGER perfCount;
			QueryPerformanceCounter(&perfCount);

			m_dTSyst = (double) (perfCount.QuadPart - m_startPerfCount) / (double) m_perfCounterFrequency;
			m_dT = (float) (m_dTSyst - m_dTSystPrev);
			if (m_dT > (1.f / 60.f))
				m_dT = (1.f / 60.f);
		}

		MSG msg = {};
		while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				isRunning = false;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		if (!isRunning)
			break;

		// Read input
		// BB could use GetKeyboard state instead to get this all at once

		for (int iVk = 0; iVk < DIM(aVkCompute); iVk++)
		{
			int vk = aVkCompute[iVk];
			bool fDown = ((GetKeyState(vk) & (1 << 15)) != 0) && m_fWindowFocused;
			if (m_mpVkFDown[vk] != fDown)
			{
				m_mpVkFDown[vk] = fDown;
				if (fDown)
				{
					m_mpVkFJustPressed[vk] = true;
				}
				else
				{
					m_mpVkFJustReleased[vk] = true;
				}
			}
		}

		if (m_fDidWindowResize)
		{
			m_pD3ddevicecontext->OMSetRenderTargets(0, 0, 0);
			m_pD3dframebufferview->Release();
			m_pD3ddepthstencilview->Release();

			HRESULT res = m_pD3dswapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			assert(SUCCEEDED(res));

			ID3D11Texture2D * d3d11FrameBuffer;
			res = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
			assert(SUCCEEDED(res));

			res = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, nullptr, &m_pD3dframebufferview);
			assert(SUCCEEDED(res));

			D3D11_TEXTURE2D_DESC depthBufferDesc;
			d3d11FrameBuffer->GetDesc(&depthBufferDesc);

			d3d11FrameBuffer->Release();

			depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			ID3D11Texture2D* depthBuffer;
			m_pD3ddevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

			m_pD3ddevice->CreateDepthStencilView(depthBuffer, nullptr, &m_pD3ddepthstencilview);

			depthBuffer->Release();

			m_fDidWindowResize = false;
		}

		float2 vecWinSize = VecWinSize();

		if (m_mpVkFDown[VK_ESCAPE])
			DestroyWindow(m_hwnd);

		// Update shader hotloading

		for (SShader * pShader : m_arypShader)
		{
			pShader->UpdateHotload();
		}

		////////// GAMEPLAY CODE (JANK)
	
		g_game.m_hCamera3DShadow->SetPosWorld(m_hSun->PosWorld());
		g_game.m_hCamera3DShadow->SetQuatWorld(QuatLookAt(-m_hSun->VecZWorld(),m_hSun->VecYWorld()));

		m_hPlaneTest2->SetQuatLocal(QuatAxisAngle(g_vecYAxis, m_dT * 10.0f) * m_hPlaneTest2->QuatLocal());

		///////////////////////////////

		// Run update functions on all nodes

		// NOTE objects spawned by update will not update or render until the next frame

		struct SVisitNode
		{
			SNodeHandle m_hNode;
			bool m_fVisited;
		};

		std::vector<SNodeHandle> aryhNode;
		std::vector<SVisitNode> aryhNodeStack;
		aryhNodeStack.push_back({ m_hNodeRoot, false });
		while (aryhNodeStack.size() > 0)
		{
			SVisitNode visitnode = aryhNodeStack[aryhNodeStack.size() - 1];
			aryhNodeStack.pop_back();

			if (visitnode.m_fVisited)
			{
				if (visitnode.m_hNode->m_hNodeSiblingNext != -1)
				{
					aryhNodeStack.push_back({ visitnode.m_hNode->m_hNodeSiblingNext, false });
				}
			}
			else
			{
				aryhNode.push_back(visitnode.m_hNode);

				aryhNodeStack.push_back({ visitnode.m_hNode, true});
				if (visitnode.m_hNode->m_hNodeChildFirst != -1)
				{
					aryhNodeStack.push_back({ visitnode.m_hNode->m_hNodeChildFirst, false });
				}
			}
		}

		std::vector<SUiNode *> arypUinodeToRender;
		std::vector<SDrawNode3D *> arypDrawnode3DToRender;
		{
			std::vector<SUiNodeHandle> aryhUinodeToRender;
			std::vector<SDrawNode3DHandle> aryhDrawnode3DToRender;

			// BB I think eventually we'd want render all draw nodes once per camera

			for (SNodeHandle hNode : aryhNode)
			{
				if (hNode == -1)
					continue;

				SNode * pNode = hNode.PT();
				pNode->Update();

				if (pNode->FIsDerivedFrom(TYPEK_UiNode))
				{
					aryhUinodeToRender.push_back(SUiNodeHandle(hNode.m_id));
				}
				else if (pNode->FIsDerivedFrom(TYPEK_DrawNode3D))
				{
					SDrawNode3D * pDrawnode = reinterpret_cast<SDrawNode3D *>(pNode);
					if (pDrawnode->m_hMaterial != -1 && pDrawnode->m_hMesh != -1)
					{
						aryhDrawnode3DToRender.push_back(SDrawNode3DHandle(hNode.m_id));
					}
				}
			}

			// Cache out the pointers so we don't have to look them up since they can't be destroyed while rendering
			//  i.e. we're done updating

			for (SUiNodeHandle & hUinode : aryhUinodeToRender)
			{
				if (SUiNode * pUinode = hUinode.PT())
				{
					arypUinodeToRender.push_back(pUinode);
				}
			}

			for (SDrawNode3DHandle & hDrawnode : aryhDrawnode3DToRender)
			{
				if (SDrawNode3D * pDrawnode = hDrawnode.PT())
				{
					arypDrawnode3DToRender.push_back(pDrawnode);
				}
			}
		}

		// Sort UI nodes

		std::qsort(arypUinodeToRender.data(), arypUinodeToRender.size(), sizeof(SUiNodeHandle *), SortUinodeRenderOrder);

		// Update skybox transform

		// NOTE if this was a node that did this work in update, it would have to update after the camera

		// Update console text

		SConsole * pConsole = m_hConsole.PT();
		pConsole->m_hTextConsole->SetText(pConsole->StrPrint());

		// Pack all the meshes into a big index and vertex buffer
		//  See https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map
		//  "A common use of these two flags involves filling dynamic index/vertex buffers with geometry that can be seen from the camera's current position. 
		//  The first time that data is entered into the buffer on a given frame, Map is called with D3D11_MAP_WRITE_DISCARD; 
		//  doing so invalidates the previous contents of the buffer. The buffer is then filled with all available data."

		// Reset all mesh flags 
		// BB Do this clearing in a better way, which would probably involve looping over meshes only

		for (SDrawNode3D * pDrawnode3D : arypDrawnode3DToRender)
		{
			SMesh3D * pMesh = pDrawnode3D->m_hMesh.PT();
			pMesh->m_iIndexdata = -1;
			pMesh->m_iVertdata = -1;
		}

		for (SUiNode * pUinode : arypUinodeToRender)
		{
			SMesh3D * pMesh = pUinode->m_hMesh.PT();

			// Don't draw ui nodes that don't have meshes

			if (!pMesh)
				continue;

			pMesh->m_iIndexdata = -1;
			pMesh->m_iVertdata = -1;
		}

		int iBIndex = 0;
		int iBVert3D = 0;

		D3D11_MAPPED_SUBRESOURCE mappedSubresourceIndex;
		g_game.m_pD3ddevicecontext->Map(m_cbufferIndex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceIndex);

		D3D11_MAPPED_SUBRESOURCE mappedSubresourceVerts3D;
		g_game.m_pD3ddevicecontext->Map(m_cbufferVertex3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceVerts3D);

		for (SDrawNode3D * pDrawnode3D : arypDrawnode3DToRender)
		{
			SMesh3D * pMesh = pDrawnode3D->m_hMesh.PT();
			if (pMesh->m_iVertdata != -1)
				continue;

			pMesh->m_cVerts = pMesh->m_aryVertdata.size();
			pMesh->m_iVertdata = iBVert3D / sizeof(SVertData3D);
			pMesh->m_cIndicies = pMesh->m_aryIIndex.size();
			pMesh->m_iIndexdata = iBIndex / sizeof(unsigned short);

			unsigned int cBVert = sizeof(SVertData3D) * pMesh->m_aryVertdata.size();
			memcpy((char *) mappedSubresourceVerts3D.pData + iBVert3D, pMesh->m_aryVertdata.data(), cBVert);
			iBVert3D += cBVert;

			unsigned int cBIndex = sizeof(unsigned short) * pMesh->m_aryIIndex.size();
			memcpy((char *) mappedSubresourceIndex.pData + iBIndex, pMesh->m_aryIIndex.data(), cBIndex);
			iBIndex += cBIndex;
		}

		for (SUiNode * pUinode : arypUinodeToRender)
		{
			SMesh3D * pMesh = pUinode->m_hMesh.PT();

			// Again skip uinodes that don't have meshes

			if (!pMesh)
				continue;

			if (pMesh->m_iVertdata != -1)
				continue;

			pMesh->m_cVerts = pMesh->m_aryVertdata.size();
			pMesh->m_iVertdata = iBVert3D / sizeof(SVertData3D);
			pMesh->m_cIndicies = pMesh->m_aryIIndex.size();
			pMesh->m_iIndexdata = iBIndex / sizeof(unsigned short);

			unsigned int cBVert = sizeof(SVertData3D) * pMesh->m_aryVertdata.size();
			memcpy((char *) mappedSubresourceVerts3D.pData + iBVert3D, pMesh->m_aryVertdata.data(), cBVert);
			iBVert3D += cBVert;

			unsigned int cBIndex = sizeof(unsigned short) * pMesh->m_aryIIndex.size();
			memcpy((char *) mappedSubresourceIndex.pData + iBIndex, pMesh->m_aryIIndex.data(), cBIndex);
			iBIndex += cBIndex;
		}

		g_game.m_pD3ddevicecontext->Unmap(m_cbufferIndex, 0);
		g_game.m_pD3ddevicecontext->Unmap(m_cbufferVertex3D, 0);

		// Start rendering stuff

		for (int i = 0; i < 2; i++)
		{
			// Clear bound render targets

			m_pD3ddevicecontext->OMSetRenderTargets(0, nullptr, nullptr);

			// Set viewport 

			D3D11_VIEWPORT viewport;
			if (i==0)
				viewport = { 0.0f, 0.0f, nShadowRes, nShadowRes, 0.0f, 1.0f };
			else
				viewport = { 0.0f, 0.0f, vecWinSize.m_x, vecWinSize.m_y, 0.0f, 1.0f };
			m_pD3ddevicecontext->RSSetViewports(1, &viewport);

			SCamera3D * pCamera3D = (i == 0 ? m_hCamera3DShadow : m_hCamera3DMain).PT();

			ID3D11RenderTargetView * pD3drtview = (i == 0 ? m_pD3dframebufferviewShadow : m_pD3dframebufferview);
			ID3D11DepthStencilView * pD3ddepthstencilview = (i == 0 ? m_pD3ddepthstencilviewShadow : m_pD3ddepthstencilview);

			FLOAT backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			m_pD3ddevicecontext->ClearRenderTargetView(pD3drtview, backgroundColor);

			//m_pD3ddevicecontext->OMSetRenderTargets(1, &pD3drtview, nullptr);

			// Clear depth to 0 since 0=far 1=near in our clip space

			m_pD3ddevicecontext->ClearDepthStencilView(pD3ddepthstencilview, D3D11_CLEAR_DEPTH, 0.0f, 0);

			BindGlobalsForCamera(pCamera3D, m_hCamera3DShadow.PT());

			// Set render target

			m_pD3ddevicecontext->OMSetRenderTargets(1, &pD3drtview, pD3ddepthstencilview);

			// Draw skybox

			if (i==1)
			{
				// TODO consider grouping together into some sort of fullscreen pass system

				const SMaterial & material = *(m_hMaterialSkybox.PT());
				const SShader & shader = *(m_hShaderSkybox.PT());
				if (shader.m_shaderk != SHADERK_Error)
				{
					ASSERT(shader.m_shaderk == SHADERK_Skybox);

					Mat matModelSkybox;
					{
						float x = Lerp(pCamera3D->m_xNearClip, pCamera3D->m_xFarClip, 0.1f);
						Mat matTranslate = MatTranslate(x * pCamera3D->MatObjectToWorld().VecX() + pCamera3D->MatObjectToWorld().Pos());
						Quat quat = QuatLookAt(-pCamera3D->VecXWorld(), pCamera3D->VecZWorld());
						Mat matRot = MatRotate(quat);
						float w = x * GTan(pCamera3D->m_radFovHorizontal * 0.5f);
						float h = w * vecWinSize.m_y / vecWinSize.m_x;
						Mat matScale = MatScale(Vector(1.0f, w, h));
						matModelSkybox = matScale * matRot * matTranslate;
					}

					m_pD3ddevicecontext->RSSetState(shader.m_data.m_pD3drasterizerstate);
					m_pD3ddevicecontext->OMSetDepthStencilState(shader.m_data.m_pD3ddepthstencilstate, 0);

					const SMesh3D & mesh = *(m_hMeshQuad.PT());

					m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pD3ddevicecontext->IASetInputLayout(shader.m_data.m_pD3dinputlayout);

					m_pD3ddevicecontext->VSSetShader(shader.m_data.m_pD3dvertexshader, nullptr, 0);
					m_pD3ddevicecontext->PSSetShader(shader.m_data.m_pD3dfragshader, nullptr, 0);

					ID3D11Buffer * aD3dbuffer[] = { m_cbufferDrawnode3D, m_cbufferGlobals };
					m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
					m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

					unsigned int cbVert = sizeof(SVertData3D);
					unsigned int s_cbMeshOffset = 0;

					m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbufferVertex3D, &cbVert, &s_cbMeshOffset);	// BB don't constantly do this
					m_pD3ddevicecontext->IASetIndexBuffer(m_cbufferIndex, DXGI_FORMAT_R16_UINT, 0);					//  ...

					D3D11_MAPPED_SUBRESOURCE mappedSubresource;
					m_pD3ddevicecontext->Map(m_cbufferDrawnode3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
					SDrawNodeRenderConstants * pDrawnode3Drc = (SDrawNodeRenderConstants *) (mappedSubresource.pData);
					pDrawnode3Drc->FillOut(matModelSkybox, pCamera3D->MatWorldToClip());

					m_pD3ddevicecontext->Unmap(m_cbufferDrawnode3D, 0);

					BindMaterialTextures(&material, &shader);

					m_pD3ddevicecontext->DrawIndexed(mesh.m_cIndicies, mesh.m_iIndexdata, mesh.m_iVertdata);
				}
			}

			// Draw 3d nodes

			Draw3D(&arypDrawnode3DToRender, pCamera3D, i == 0);

			// Draw ui nodes
			if (i == 1)
			{
				for (SUiNode * pUinode : arypUinodeToRender)
				{
					if (pUinode->m_hMaterial == nullptr)
						continue;

					const SMaterial & material = *(pUinode->m_hMaterial);
					const SShader & shader = *(material.m_hShader);
					if (shader.m_shaderk == SHADERK_Error)
						continue;

					ASSERT(shader.m_shaderk == SHADERK_Ui);

					const SMesh3D & mesh = *pUinode->m_hMesh;

					m_pD3ddevicecontext->IASetInputLayout(shader.m_data.m_pD3dinputlayout);
					m_pD3ddevicecontext->VSSetShader(shader.m_data.m_pD3dvertexshader, nullptr, 0);
					m_pD3ddevicecontext->PSSetShader(shader.m_data.m_pD3dfragshader, nullptr, 0);

					float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
					UINT sampleMask = 0xffffffff;
					m_pD3ddevicecontext->OMSetBlendState(shader.m_data.m_pD3dblendstatenoblend, blendFactor, sampleMask);
					m_pD3ddevicecontext->RSSetState(shader.m_data.m_pD3drasterizerstate);
					m_pD3ddevicecontext->OMSetDepthStencilState(shader.m_data.m_pD3ddepthstencilstate, 0);

					m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					ID3D11Buffer * aD3dbuffer[] = { m_cbufferUiNode, m_cbufferGlobals };
					m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
					m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

					unsigned int cbVert = sizeof(SVertData3D);
					unsigned int s_cbMeshOffset = 0;

					m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbufferVertex3D, &cbVert, &s_cbMeshOffset);	// BB don't constantly do this
					m_pD3ddevicecontext->IASetIndexBuffer(m_cbufferIndex, DXGI_FORMAT_R16_UINT, 0);					//  ...

					D3D11_MAPPED_SUBRESOURCE mappedSubresource;
					m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
					SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
					pUinode->GetRenderConstants(pUinoderc);
					m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

					BindMaterialTextures(&material, &shader);

					m_pD3ddevicecontext->DrawIndexed(mesh.m_cIndicies, mesh.m_iIndexdata, mesh.m_iVertdata);
				}
			}
		}

		m_pD3dswapchain->Present(1, 0);
			
		for (int i = 0; i < DIM(m_mpVkFJustPressed); i++)
		{
			m_mpVkFJustPressed[i] = false;
			m_mpVkFJustReleased[i] = false;
		}

		if (m_fWindowFocused)
		{
			float2 vecWinTopLeft = VecWinTopLeft();
			SetCursorPos(vecWinTopLeft.m_x + vecWinSize.m_x / 2, vecWinTopLeft.m_y + vecWinSize.m_y / 2);
		}

		m_sScroll = 0.0f;
	}
}

void SGame::PrintConsole(const std::string & str, float dT)
{
	m_hConsole->Print(str, g_game.m_dTSyst + dT);
}

void SGame::VkPressed(int vk)
{
   ASSERT(vk> 0 && vk < DIM(m_mpVkFDown));
   if (!m_mpVkFDown[vk])
		   m_mpVkFJustPressed[vk] = true;
   m_mpVkFDown[vk] = true;
}

void SGame::VkReleased(int vk)
{
   ASSERT(vk> 0 && vk < DIM(m_mpVkFDown));
   if (m_mpVkFDown[vk])
		   m_mpVkFJustReleased[vk] = true;
   m_mpVkFDown[vk] = false;
}

LRESULT SGame::LresultWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
		/*
		case WM_INPUT: 
		{
			// https://learn.microsoft.com/en-us/windows/win32/inputdev/using-raw-input

			// TODO use raw input for keyboard too, see above link

			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpb[sizeof(RAWINPUT)];

			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;
			
			if (raw->header.dwType == RIM_TYPEMOUSE) 
			{
				if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
				{
					m_xCursor = raw->data.mouse.lLastX;
					m_yCursor = raw->data.mouse.lLastY;
				}

				if (raw->data.mouse.usFlags & MOUSE_MOVE_RELATIVE)
				{
					m_xCursor += raw->data.mouse.lLastX;
					m_yCursor += raw->data.mouse.lLastY;
				}

				if ((raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) != 0)
				{
					VkPressed(VK_LBUTTON);
				}

				if ((raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) != 0)
				{
					VkReleased(VK_LBUTTON);
				}

				if ((raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) != 0)
				{
					VkPressed(VK_RBUTTON);
				}

				if ((raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) != 0)
				{
					VkReleased(VK_RBUTTON);
				}
			} 

			return 0;
		} 
		*/

		case WM_MOUSEMOVE:
			{
				short nLower = lparam;
				short nUpper = lparam >> 16;
				m_xCursor = nLower;
				m_yCursor = nUpper;
			}
			break;
		case WM_MOUSEWHEEL:
			{
				short nUpper = lparam;
				m_sScroll = GET_WHEEL_DELTA_WPARAM(wparam);
			}
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
			// We manually read keys
			break;

		case WM_SETFOCUS:
			m_fWindowFocused = true;
			return 0;

		case WM_KILLFOCUS:
			m_fWindowFocused = false;
			return 0;

		case WM_SYSKEYDOWN:
			// Don't do default to avoid tapping alt freezing the game
			break;

		// https://learn.microsoft.com/en-us/windows/win32/menurc/keyboard-accelerators
		case WM_SYSCHAR:
			// Disable beep when you type alt+another key that (the missing accelerator warning noise)
			break;

		case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			break;

		case WM_SIZE:
			{
				m_fDidWindowResize = true;
			}
			break;

		default:
			result = DefWindowProcW(hwnd, msg, wparam, lparam);
			break;
	}
	return result;
}
