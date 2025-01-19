// Uses some d3d api code from https://github.com/kevinmoran/BeginnerDirect3D11

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine.h"
#include "fpscounter.h"
#include "texture.h"
#include "shader.h"
#include "flycam.h"

#include <exception>

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

SGame::SGame()
{
	for (int i = 0; i < DIM(m_mpVkFDown); i++)
	{
		m_mpVkFDown[i] = false;
		m_mpVkFJustPressed[i] = false;
		m_mpVkFJustReleased[i] = false;
	}
}

void SGame::Init(HINSTANCE hInstance)
{
	AuditFixArray();
	AuditVectors();
	AuditSlotheap();
	AuditNFromStr();

	m_hNodeRoot = (new SNode(-1, "RootNode"))->HNode();

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

		m_hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
			winClass.lpszClassName,
			L"Engine",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			initialWidth,
			initialHeight,
			0, 0, hInstance, 0);

		if (!m_hwnd)
		{
			MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
			exit;
		}
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
#if OLD
		ID3D11Texture2D * d3d11FrameBuffer;
		HRESULT hResult = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
		assert(SUCCEEDED(hResult));

		hResult = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, 0, &m_pD3dframebufferview);
		assert(SUCCEEDED(hResult));
		d3d11FrameBuffer->Release();
#endif

		////////////////////////

		// BB overlap/duplication with resize code

		ID3D11Texture2D * d3d11FrameBuffer;
		HRESULT hResult = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
		assert(SUCCEEDED(hResult));

		hResult = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, nullptr, &m_pD3dframebufferview);
		assert(SUCCEEDED(hResult));

		D3D11_TEXTURE2D_DESC depthBufferDesc;
		d3d11FrameBuffer->GetDesc(&depthBufferDesc);

		d3d11FrameBuffer->Release();

		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D* depthBuffer;
		m_pD3ddevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

		m_pD3ddevice->CreateDepthStencilView(depthBuffer, nullptr, &m_pD3ddepthstencilview);

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
		const int cVertsMax = 100000;

		D3D11_BUFFER_DESC descVertexBuffer = {};
		descVertexBuffer.ByteWidth = cVertsMax * sizeof(SVertData2D);
		descVertexBuffer.Usage = D3D11_USAGE_DYNAMIC;
		descVertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descVertexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descVertexBuffer, nullptr, &m_cbufferVertex2D);
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

	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		//rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FrontCounterClockwise = TRUE;

		m_pD3ddevice->CreateRasterizerState(&rasterizerDesc, &m_pD3drasterizerstate);
	}

	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		m_pD3ddevice->CreateDepthStencilState(&depthStencilDesc, &m_pD3ddepthstencilstate);
	}

	{
		D3D11_BLEND_DESC1 BlendState;
		ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
		D3D11_RENDER_TARGET_BLEND_DESC1 * pD3drtbd = &BlendState.RenderTarget[0];
		pD3drtbd->BlendEnable = TRUE;
		pD3drtbd->SrcBlend = D3D11_BLEND_SRC_ALPHA;
		pD3drtbd->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		pD3drtbd->BlendOp = D3D11_BLEND_OP_ADD;
		pD3drtbd->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		pD3drtbd->BlendOpAlpha = D3D11_BLEND_OP_ADD;
		pD3drtbd->SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		pD3drtbd->DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

		pD3drtbd->LogicOpEnable = FALSE;
		pD3drtbd->LogicOp = D3D11_LOGIC_OP_CLEAR;

		m_pD3ddevice->CreateBlendState1(&BlendState, &m_pD3dblendstatenoblend);
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

	SShaderHandle hShaderUnlit = (new SShader("shaders\\unlit2d.hlsl"))->HShader();
	SShaderHandle hShaderText = (new SShader("shaders\\text2d.hlsl"))->HShader();

	SShaderHandle hShader3D = (new SShader("shaders\\unlit3d.hlsl"))->HShader();

	// Font 

	m_hFont = (new SFont("fonts\\candara.fnt"))->HFont();

	m_hMaterialText = (new SMaterial(hShaderText))->HMaterial();
	//m_hMaterialText->m_hTexture = m_hFont->m_aryhTexture[0];
	m_hMaterialText->m_aryNamedtexture.push_back({ m_hFont->m_aryhTexture[0], "fontTexture" });

	// Console text

	m_hTextConsole = (new SText(m_hFont, m_hNodeRoot, "ConsoleText"))->HText();
	m_hTextConsole->m_hMaterial = m_hMaterialText;
	m_hTextConsole->m_vecScale = float2(0.2f, 0.2f);
	m_hTextConsole->m_gSort = 10.0f;
	m_hTextConsole->m_pos = float2(20.0f, 500.0f);
	m_hTextConsole->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Fps counter

	(new SFpsCounter(m_hNodeRoot, "FpsCounter"));

	// Camera

	//m_hCamera3DMain = (new SCamera3D(m_hNodeRoot, "CameraMain", RadFromDeg(90.0f), 0.1, 100.0f))->HCamera3D();

	(new SFlyCam(m_hNodeRoot, "FlyCam"));

	SMaterial * pMaterial3d = new SMaterial(hShader3D);
	pMaterial3d->m_aryNamedtexture.push_back({ (new STexture("textures/testTexture1.png", false, false))->HTexture(),  "mainTexture"});
	pMaterial3d->m_aryNamedtexture.push_back({ (new STexture("textures/testTexture2.png", false, false))->HTexture(),  "altTexture"});

	m_hPlaneTest = (new SDrawNode3D(m_hNodeRoot, "PlaneTest1"))->HDrawnode3D();
	m_hPlaneTest->m_hMaterial = pMaterial3d->HMaterial();
	m_hPlaneTest->SetPosWorld(Point(10.0f, 0.0f, 0.0f));
	m_hPlaneTest->m_hMesh = m_hMeshQuad;

	m_hPlaneTest2 = (new SDrawNode3D(m_hPlaneTest->HNode(), "PlaneTest2"))->HDrawnode3D();
	m_hPlaneTest2->m_hMaterial = pMaterial3d->HMaterial();
	m_hPlaneTest2->SetPosWorld(Point(10.0f, 2.0f, 0.0f));
	m_hPlaneTest2->m_hMesh = m_hMeshQuad;
}

int SortUinodeRenderOrder(const void * pVa, const void * pVb)
{
	SUiNodeHandle hUinodeA = *(SUiNodeHandle*) pVa;
	SUiNodeHandle hUinodeB = *(SUiNodeHandle*) pVb;
	if (hUinodeA->m_gSort == hUinodeB->m_gSort)
		return 0;
	else if (hUinodeA->m_gSort < hUinodeB->m_gSort)
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

		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, vecWinSize.m_x, vecWinSize.m_y, 0.0f, 1.0f };

		if (m_mpVkFDown[VK_ESCAPE])
			DestroyWindow(m_hwnd);

		////////// GAMEPLAY CODE (JANK)
	
		//m_hPlaneTest->m_transformLocal.m_quat = QuatAxisAngle(g_vecZAxis, m_dT*10.0f) * m_hPlaneTest->m_transformLocal.m_quat;
		m_hPlaneTest->SetQuatLocal(QuatAxisAngle(g_vecZAxis, m_dT) * m_hPlaneTest->QuatLocal());
		m_hPlaneTest2->SetQuatLocal(QuatAxisAngle(g_vecYAxis, m_dT * 10.0f) * m_hPlaneTest2->QuatLocal());
		//m_hPlaneTest->m_transformLocal.m_quat = QuatAxisAngle(g_vecZAxis, *m_dT) * m_hPlaneTest->m_transformLocal.m_quat;

		///////////////////////////////

		m_strConsole = "";

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
				aryhDrawnode3DToRender.push_back(SDrawNode3DHandle(hNode.m_id));
			}
		}
		
		m_hTextConsole->SetText(m_strConsole);

		// Sort UI nodes

		std::qsort(aryhUinodeToRender.data(), aryhUinodeToRender.size(), sizeof(SUiNodeHandle), SortUinodeRenderOrder);

		// Start rendering stuff

		SCamera3D * pCamera3D = m_hCamera3DMain.PT();

		FLOAT backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_pD3ddevicecontext->ClearRenderTargetView(m_pD3dframebufferview, backgroundColor);
		m_pD3ddevicecontext->RSSetViewports(1, &viewport);
		m_pD3ddevicecontext->OMSetRenderTargets(1, &m_pD3dframebufferview, nullptr);
		m_pD3ddevicecontext->ClearDepthStencilView(m_pD3ddepthstencilview, D3D11_CLEAR_DEPTH, 1.0f, 0);

		{
			D3D11_MAPPED_SUBRESOURCE mappedSubresourceGlobals;
			m_pD3ddevicecontext->Map(m_cbufferGlobals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceGlobals);
			ShaderGlobals * pShaderglobals = (ShaderGlobals *) (mappedSubresourceGlobals.pData);
			pShaderglobals->m_t = m_dTSyst;
			pShaderglobals->m_vecWinSize = vecWinSize;
			m_pD3ddevicecontext->Unmap(m_cbufferGlobals, 0);
		}

		// Pack all the meshes into a big index and vertex buffer
		//  See https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map
		//  "A common use of these two flags involves filling dynamic index/vertex buffers with geometry that can be seen from the camera's current position. 
		//  The first time that data is entered into the buffer on a given frame, Map is called with D3D11_MAP_WRITE_DISCARD; 
		//  doing so invalidates the previous contents of the buffer. The buffer is then filled with all available data."

		// Reset all mesh flags 
		// BB Do this clearing in a better way, which would probably involve looping over meshes only

		for (SDrawNode3DHandle hDrawnode3D : aryhDrawnode3DToRender)
		{
			SMesh3D * pMesh = hDrawnode3D->m_hMesh.PT();
			pMesh->m_iIndexdata = -1;
			pMesh->m_iVertdata = -1;
		}

		for (SUiNodeHandle hUinode : aryhUinodeToRender)
		{
			SMesh2D * pMesh = hUinode->m_hMesh.PT();
			pMesh->m_iIndexdata = -1;
			pMesh->m_iVertdata = -1;
		}

		int iBIndex = 0;
		int iBVert3D = 0;
		int iBVert2D = 0;

		D3D11_MAPPED_SUBRESOURCE mappedSubresourceIndex;
		g_game.m_pD3ddevicecontext->Map(m_cbufferIndex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceIndex);

		D3D11_MAPPED_SUBRESOURCE mappedSubresourceVerts3D;
		g_game.m_pD3ddevicecontext->Map(m_cbufferVertex3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceVerts3D);

		D3D11_MAPPED_SUBRESOURCE mappedSubresourceVerts2D;
		g_game.m_pD3ddevicecontext->Map(m_cbufferVertex2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceVerts2D);

		for (SDrawNode3DHandle hDrawnode3D : aryhDrawnode3DToRender)
		{
			SMesh3D * pMesh = hDrawnode3D->m_hMesh.PT();
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

		for (SUiNodeHandle hUinode : aryhUinodeToRender)
		{
			SMesh2D * pMesh = hUinode->m_hMesh.PT();
			if (pMesh->m_iVertdata != -1)
				continue;

			pMesh->m_cVerts = pMesh->m_aryVertdata.size();
			pMesh->m_iVertdata = iBVert2D / sizeof(SVertData2D);
			pMesh->m_cIndicies = pMesh->m_aryIIndex.size();
			pMesh->m_iIndexdata = iBIndex / sizeof(unsigned short);

			unsigned int cBVert = sizeof(SVertData2D) * pMesh->m_aryVertdata.size();
			memcpy((char *) mappedSubresourceVerts2D.pData + iBVert2D, pMesh->m_aryVertdata.data(), cBVert);
			iBVert2D += cBVert;

			unsigned int cBIndex = sizeof(unsigned short) * pMesh->m_aryIIndex.size();
			memcpy((char *) mappedSubresourceIndex.pData + iBIndex, pMesh->m_aryIIndex.data(), cBIndex);
			iBIndex += cBIndex;
		}

		g_game.m_pD3ddevicecontext->Unmap(m_cbufferIndex, 0);
		g_game.m_pD3ddevicecontext->Unmap(m_cbufferVertex3D, 0);
		g_game.m_pD3ddevicecontext->Unmap(m_cbufferVertex2D, 0);

		// Draw 3d nodes

		{
			m_pD3ddevicecontext->RSSetState(m_pD3drasterizerstate);
			m_pD3ddevicecontext->OMSetDepthStencilState(m_pD3ddepthstencilstate, 0);

			m_pD3ddevicecontext->OMSetRenderTargets(1, &m_pD3dframebufferview, m_pD3ddepthstencilview);

			for (SDrawNode3DHandle hDrawnode3D : aryhDrawnode3DToRender)
			{
				if (!hDrawnode3D.PT())
					continue;

				if (hDrawnode3D->m_hMaterial == nullptr)
					continue;

				SDrawNode3D * pDrawnode3D = hDrawnode3D.PT();

				const SMaterial & material = *hDrawnode3D->m_hMaterial;
				const SShader & shader = *(material.m_hShader);
				ASSERT(hDrawnode3D->m_typek == TYPEK_DrawNode3D);
				ASSERT(shader.m_shaderk == SHADERK_3D);

				const SMesh3D & mesh = *hDrawnode3D->m_hMesh;

				m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);

				m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
				m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

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

				// TODO don't constantly recompute

				Mat matModel = hDrawnode3D->MatObjectToWorld();
				Mat matCamera = pCamera3D->MatObjectToWorld().MatInverse();
				Mat matPerspective = MatPerspective(pCamera3D->m_radFovHorizontal, vecWinSize.m_x / vecWinSize.m_y, pCamera3D->m_xNearClip, pCamera3D->m_xFarClip);
				ASSERT(sizeof(Mat) == sizeof(float) * 16);
				pDrawnode3Drc->m_matMVP = matModel * matCamera * matPerspective;

				m_pD3ddevicecontext->Unmap(m_cbufferDrawnode3D, 0);

				BindMaterialTextures(&material, &shader);

				m_pD3ddevicecontext->DrawIndexed(mesh.m_cIndicies, mesh.m_iIndexdata, mesh.m_iVertdata);
			}
		}

		// Draw ui nodes
		for (SUiNodeHandle hUinode : aryhUinodeToRender)
		{
			if (!hUinode.PT())
				continue;

			if (hUinode->m_hMaterial == nullptr)
				continue;

			const SMaterial & material = *hUinode->m_hMaterial;
			const SShader & shader = *(material.m_hShader);
			ASSERT(shader.m_shaderk == SHADERK_Ui);

			const SMesh2D & mesh = *hUinode->m_hMesh;

			m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
			m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
			m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

			float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			UINT sampleMask   = 0xffffffff;
			m_pD3ddevicecontext->OMSetBlendState(m_pD3dblendstatenoblend, blendFactor, sampleMask);

			m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ID3D11Buffer * aD3dbuffer[] = { m_cbufferUiNode, m_cbufferGlobals };
			m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
			m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

			unsigned int cbVert = sizeof(SVertData2D);
			unsigned int s_cbMeshOffset = 0;

			m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbufferVertex2D, &cbVert, &s_cbMeshOffset);	// BB don't constantly do this
			m_pD3ddevicecontext->IASetIndexBuffer(m_cbufferIndex, DXGI_FORMAT_R16_UINT, 0);					//  ...

			D3D11_MAPPED_SUBRESOURCE mappedSubresource;
			m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
			SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
			hUinode->GetRenderConstants(pUinoderc);
			m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

			BindMaterialTextures(&material, &shader);

			m_pD3ddevicecontext->DrawIndexed(mesh.m_cIndicies, mesh.m_iIndexdata, mesh.m_iVertdata);
		}

		for (int i = 0; i < DIM(m_mpVkFJustPressed); i++)
		{
			m_mpVkFJustPressed[i] = false;
			m_mpVkFJustReleased[i] = false;
		}
		m_sScroll = 0.0f;

		m_pD3dswapchain->Present(1, 0);
	}
}

void SGame::BindMaterialTextures(const SMaterial * pMaterial, const SShader * pShader)
{
	ASSERT(pMaterial->m_aryNamedtexture.size() == pShader->m_mpISlotStrName.size());

	std::vector<ID3D11ShaderResourceView *> arypD3dsrview;
	std::vector<ID3D11SamplerState *> arypD3dsamplerstate;

	for (int i = 0; i < pShader->CNamedslot(); i++)
	{
		arypD3dsrview.push_back(nullptr);
		arypD3dsamplerstate.push_back(nullptr);
	}

	for (const SNamedTexture & namedtexture : pMaterial->m_aryNamedtexture)
	{
		for (const SNamedTextureSlot & namedslot : pShader->m_mpISlotStrName)
		{
			if (namedslot.m_strName == namedtexture.m_strName)
			{
				ASSERT(arypD3dsrview[namedslot.m_iSlot] == nullptr);
				ASSERT(arypD3dsamplerstate[namedslot.m_iSlot] == nullptr);
				arypD3dsrview[namedslot.m_iSlot] = namedtexture.m_hTexture->m_pD3dsrview;
				arypD3dsamplerstate[namedslot.m_iSlot] = namedtexture.m_hTexture->m_pD3dsamplerstate;
			}
		}
	}

	for (int i = 0; i < pShader->CNamedslot(); i++)
	{
		ASSERT(arypD3dsrview[i] != nullptr);
		ASSERT(arypD3dsamplerstate[i] != nullptr);
	}

	m_pD3ddevicecontext->PSSetShaderResources(0, arypD3dsrview.size(), arypD3dsrview.data());
	m_pD3ddevicecontext->PSSetSamplers(0, arypD3dsamplerstate.size(), arypD3dsamplerstate.data());
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

void SGame::PrintConsole(const std::string & str)
{
	m_strConsole += str;
}

LRESULT SGame::LresultWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
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
			{
				VkPressed(VK_RBUTTON);
			}
			break;

		case WM_RBUTTONUP:
			{
				VkReleased(VK_RBUTTON);
			}
			break;

		case WM_LBUTTONDOWN:
			{
				VkPressed(VK_LBUTTON);
			}
			break;

		case WM_LBUTTONUP:
			{
				VkReleased(VK_LBUTTON);
			}
			break;

		case WM_KEYDOWN:
			{
				VkPressed(wparam);
			}
			break;

		case WM_KEYUP:
			{
				VkReleased(wparam);
			}
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
