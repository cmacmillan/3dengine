// Uses some d3d api code from https://github.com/kevinmoran/BeginnerDirect3D11

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine.h"

#include <exception>

SGame g_game;
SObjectManager g_objman;

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

// SCBufferAllocator 

SSlotHeap<SCBufferAllocatorNode> g_slotheapCbanode;

SCBufferAllocatorNode::SCBufferAllocatorNode(int ibStart, int cbFree)
{ 
	m_slice = { ibStart, cbFree };
}

void SCBufferAllocatorNode::operator delete(void * ptr)
{
	g_slotheapCbanode.FreePT((SCBufferAllocatorNode *) ptr);
}

SCBufferAllocator::SCBufferAllocator(const D3D11_BUFFER_DESC & desc, int cbAlignment)
{
	ASSERT(desc.ByteWidth % cbAlignment == 0);

	m_desc = desc;
	m_cbAlignment = cbAlignment;
	HRESULT hResult = g_game.m_pD3ddevice->CreateBuffer(&m_desc, nullptr, &m_cbuffer);
	ASSERT(SUCCEEDED(hResult));
	m_pCbanodeStart = new (g_slotheapCbanode.PTAlloc()) SCBufferAllocatorNode(0, m_desc.ByteWidth);
}

SCBufferSlice SCBufferAllocator::SliceClaim(int cbDesired)
{
	// NOTE we don't need to worry about maintaining alignment,
	//  since we force all requests to request aligned amount of memory

	ASSERT(cbDesired % m_cbAlignment == 0);
	ASSERT(cbDesired <= m_desc.ByteWidth);

	SCBufferAllocatorNode * pCbanodePrev = nullptr;
	for (SCBufferAllocatorNode * pCbanode = m_pCbanodeStart; pCbanode; pCbanode = pCbanode->m_pCbanodeNext)
	{
		if (pCbanode->m_slice.m_cb >= cbDesired)
		{
			if (pCbanode->m_slice.m_cb == cbDesired)
			{
				// Complete match, totally remove node from list

				if (pCbanodePrev)
				{
					pCbanodePrev->m_pCbanodeNext = pCbanode->m_pCbanodeNext;
				}
				else
				{
					m_pCbanodeStart = pCbanode->m_pCbanodeNext;
				}

				int ibResult = pCbanode->m_slice.m_ibStart;
				delete pCbanode;
				return { ibResult, cbDesired };
			}
			else
			{
				// Too much memory, shrink node

				int ibResult = pCbanode->m_slice.m_ibStart;
				pCbanode->m_slice.m_cb -= cbDesired;
				pCbanode->m_slice.m_ibStart += cbDesired;
				return { ibResult, cbDesired };
			}
		}

		pCbanodePrev = pCbanode;
	}

	ASSERT(false); // couldn't find enough memory!
	return { -1, -1 };
}

bool FCanMergeSlices(const SCBufferSlice & sliceSmaller, const SCBufferSlice & sliceLarger)
{
	return sliceSmaller.m_ibStart + sliceSmaller.m_cb == sliceLarger.m_ibStart;
}

void SCBufferAllocator::ReleaseSlice(const SCBufferSlice & slice)
{
	ASSERT(slice.m_cb % m_cbAlignment == 0);
	ASSERT(slice.m_cb <= m_desc.ByteWidth);

	if (!m_pCbanodeStart)
	{
		m_pCbanodeStart = new (g_slotheapCbanode.PTAlloc()) SCBufferAllocatorNode(slice.m_ibStart, slice.m_cb);
		return;
	}

	SCBufferAllocatorNode * pCbanodePrev = nullptr;
	for (SCBufferAllocatorNode * pCbanode = m_pCbanodeStart; pCbanode; pCbanode = pCbanode->m_pCbanodeNext)
	{
		ASSERT(pCbanode->m_slice.m_ibStart != slice.m_ibStart);
		ASSERT(pCbanode->m_slice.m_ibStart + pCbanode->m_slice.m_cb <= slice.m_ibStart || slice.m_ibStart + slice.m_cb <= pCbanode->m_slice.m_ibStart);

		// if the current node is larger than use, we go before it

		if (slice.m_ibStart < pCbanode->m_slice.m_ibStart)
		{
			// Insert before

			if (pCbanodePrev)
			{
				if (FCanMergeSlices(pCbanodePrev->m_slice, slice))
				{
					// Can we also merge with next?

					if (FCanMergeSlices(slice, pCbanode->m_slice))
					{
						// Merge with both

						pCbanodePrev->m_pCbanodeNext = pCbanode->m_pCbanodeNext;
						pCbanodePrev->m_slice.m_cb += slice.m_cb + pCbanode->m_slice.m_cb;
						delete pCbanode;
						return;
					}
					else
					{
						// Only merge with prev

						pCbanodePrev->m_slice.m_cb += slice.m_cb;
						return;
					}
				}
				else
				{
					// Can't merge with prev, try to merge with next

					if (FCanMergeSlices(slice, pCbanode->m_slice))
					{
						pCbanode->m_slice.m_cb += slice.m_cb;
						pCbanode->m_slice.m_ibStart = slice.m_ibStart;				
						return;
					}
					else
					{
						// No merging possible

						SCBufferAllocatorNode * pCbanodeNew = new (g_slotheapCbanode.PTAlloc()) SCBufferAllocatorNode(slice.m_ibStart, slice.m_cb);
						pCbanodeNew->m_pCbanodeNext = pCbanodePrev->m_pCbanodeNext;
						pCbanodePrev->m_pCbanodeNext = pCbanodeNew;
						return;
					}
				}
			}
			else
			{
				// We're the first node

				if (FCanMergeSlices(slice, pCbanode->m_slice))
				{
					pCbanode->m_slice.m_cb += slice.m_cb;
					pCbanode->m_slice.m_ibStart = slice.m_ibStart;
					return;
				}
				else
				{
					m_pCbanodeStart = new (g_slotheapCbanode.PTAlloc()) SCBufferAllocatorNode(slice.m_ibStart, slice.m_cb);
					m_pCbanodeStart->m_pCbanodeNext = pCbanode;
					return;
				}	
			}
		}

		pCbanodePrev = pCbanode;
	}

	// See if we can merge with the last node

	if (FCanMergeSlices(pCbanodePrev->m_slice, slice))
	{
		pCbanodePrev->m_slice.m_cb += slice.m_cb;
	}
	else
	{
		SCBufferAllocatorNode * pCbanodeNew = new (g_slotheapCbanode.PTAlloc()) SCBufferAllocatorNode(slice.m_ibStart, slice.m_cb);
		pCbanodePrev->m_pCbanodeNext = pCbanodeNew;
	}
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

void SObjectManager::RegisterObj(SObject * pObj)
{
	int id = m_cId;
	m_cId++;
	m_mpObjhObj.emplace(id, pObj);
	pObj->m_nHandle = id;
}

void SObjectManager::UnregisterObj(SObject * pObj)
{
	m_mpObjhObj.erase(m_mpObjhObj.find(pObj->m_nHandle));
}

SObject::SObject()
{
	g_objman.RegisterObj(this);
}

SObject::~SObject()
{
	g_objman.UnregisterObj(this);
}

// BB could be faster and non-recursive

bool SObject::FIsDerivedFrom(TYPEK typek)
{
	for (TYPEK typekIter = m_typek; typekIter != TYPEK_Nil; typekIter = TypekSuper(typekIter))
	{
		if (typekIter == typek)
			return true;
	}

	return false;
}

STexture::STexture(const char * pChzFilename, bool fIsNormal, bool fGenerateMips) : super()
{
	m_typek = TYPEK_Texture;

	// Create Sampler State

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

#if ENABLE_ANISOTROPY
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
#endif

	g_game.m_pD3ddevice->CreateSamplerState(&samplerDesc, &m_pD3dsamplerstate);

	std::string strPath = StrPrintf("%s%s", ASSET_PATH, pChzFilename);

	// Load Image
	int texNumChannels;
	int texForceNumChannels = 4;
	unsigned char * aBTexture = stbi_load(strPath.c_str(), &m_dX, &m_dY,
		&texNumChannels, texForceNumChannels);
	assert(aBTexture);
	int texBytesPerRow = 4 * m_dX;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_dX;
	textureDesc.Height = m_dY;
	//textureDesc.MipLevels = 1;
	textureDesc.MipLevels = fGenerateMips ? 0 : 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = fIsNormal ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	//textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (fGenerateMips)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
	textureSubresourceData.pSysMem = aBTexture;
	textureSubresourceData.SysMemPitch = texBytesPerRow;
	textureSubresourceData.SysMemSlicePitch = 4 * m_dX * m_dY;

	HRESULT hresultTexture = g_game.m_pD3ddevice->CreateTexture2D(&textureDesc, fGenerateMips ? nullptr : &textureSubresourceData, &m_pD3dtexture);
	ASSERT(hresultTexture == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dsrvd = {};

	d3dsrvd.Format = textureDesc.Format;
	d3dsrvd.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	d3dsrvd.Texture2D.MipLevels = -1;

	g_game.m_pD3ddevice->CreateShaderResourceView(m_pD3dtexture, &d3dsrvd, &m_pD3dsrview);

	if (fGenerateMips)
	{
		g_game.m_pD3ddevicecontext->UpdateSubresource(m_pD3dtexture, 0, nullptr, aBTexture, 4 * m_dX, 4 * m_dX * m_dY);
		g_game.m_pD3ddevicecontext->GenerateMips(m_pD3dsrview);
	}

	free(aBTexture);
}

SShader::SShader(LPCWSTR lpcwstrFilename, bool fIs3D) : super()
{
	m_typek = TYPEK_Shader;

	std::wstring wstrPath = WstrFromStr(std::string(ASSET_PATH)) + std::wstring(lpcwstrFilename);

	// Create Vertex Shader
	ID3DBlob * vsBlob = nullptr;
	{
		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(wstrPath.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{

			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			{
				MessageBoxA(0, "Could not compile shader; file not found", "Shader Compiler Error", MB_ICONERROR | MB_OK);
			}
			else if (shaderCompileErrorsBlob)
			{
				MessageBoxA(0, (const char *) shaderCompileErrorsBlob->GetBufferPointer(), "Shader Compiler Error", MB_ICONERROR | MB_OK);
			}
			exit;
		}

		hResult = g_game.m_pD3ddevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pD3dvertexshader);
		assert(SUCCEEDED(hResult));
	}

	// Create Pixel Shader
	{
		ID3DBlob * psBlob;
		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(wstrPath.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char * errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (shaderCompileErrorsBlob)
			{
				errorString = (const char *) shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			exit;
		}

		hResult = g_game.m_pD3ddevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pD3dfragshader);
		assert(SUCCEEDED(hResult));
		psBlob->Release();
	}

	// Create Input Layout

	// BB force this to match with the input data

	if (fIs3D)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HRESULT hResult = g_game.m_pD3ddevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pD3dinputlayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HRESULT hResult = g_game.m_pD3ddevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pD3dinputlayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}
}

SMaterial::SMaterial(SShaderHandle hShader) : super()
{
	m_typek = TYPEK_Material;

	m_hShader = hShader;
}

/*
float2 SCamera::PosWorldFromCursor(float2 posCursor)
{
	float2 posBottomLeft = m_pos - m_vecExtents;
	float2 posTopRight = m_pos + m_vecExtents;
	float2 vecWinSize = g_game.VecWinSize();
	return float2(
		GMapRange(0.0f, vecWinSize.m_x, posBottomLeft.m_x, posTopRight.m_x, posCursor.m_x),
		GMapRange(vecWinSize.m_y, 0.0f, posBottomLeft.m_y, posTopRight.m_y, posCursor.m_y)); // NOTE inverted y
}
*/

char SBinaryStream::CharRead()
{
	return m_pB[m_i++];
}

unsigned char SBinaryStream::UcharRead()
{
	return m_pB[m_i++];
}

short SBinaryStream::ShortRead()
{
	short s = m_pB[m_i] | (m_pB[m_i+1] << 8);
	m_i += 2;
	return s;
}

unsigned short SBinaryStream::UshortRead()
{
	unsigned short s = m_pB[m_i] | (m_pB[m_i+1] << 8);
	m_i += 2;
	return s;
}

int SBinaryStream::IntRead()
{
	int n = m_pB[m_i] | (m_pB[m_i+1] << 8) | (m_pB[m_i+2] << 16) | (m_pB[m_i+3] << 24);
	m_i += 4;
	return n;
}

unsigned int SBinaryStream::UintRead()
{
	unsigned int n = m_pB[m_i] | (m_pB[m_i+1] << 8) | (m_pB[m_i+2] << 16) | (m_pB[m_i+3] << 24);
	m_i += 4;
	return n;
}

const char * SBinaryStream::PChzRead()
{
	const char * pChz = (const char *)m_pB + m_i;
	int c = strlen(pChz);
	m_i = m_i + c + 1;
	return pChz;
}

struct SFontBlockHeader
{
	SFontBlockHeader(SBinaryStream * pBs);
	char m_bType;
	unsigned int m_cBytes;
};

// https://www.angelcode.com/products/bmfont/doc/file_format.html#bin

SFontBlockHeader::SFontBlockHeader(SBinaryStream * pBs)
{
	m_bType = pBs->CharRead();
	m_cBytes = pBs->UintRead();
}

SFontInfoBlock::SFontInfoBlock(SBinaryStream * pBs)
{
	m_nFontSize = pBs->ShortRead();
	m_bBitField = pBs->CharRead();
	m_bCharSet = pBs->UcharRead();
	m_nStretchH = pBs->UshortRead();
	m_bAA = pBs->UcharRead();
	m_bPaddingUp = pBs->UcharRead();
	m_bPaddingRight = pBs->UcharRead();
	m_bPaddingDown = pBs->UcharRead();
	m_bPaddingLeft = pBs->UcharRead();
	m_bSpacingHoriz = pBs->UcharRead();
	m_bSpacingVert = pBs->UcharRead();
	m_bOutline = pBs->UcharRead();
}

SFontCommonBlock::SFontCommonBlock(SBinaryStream * pBs)
{
	m_nLineHeight = pBs->UshortRead();
	m_nBase = pBs->UshortRead();
	m_nScaleW = pBs->UshortRead();
	m_nScaleH = pBs->UshortRead();
	m_nPages = pBs->UshortRead();
	m_bBitField = pBs->CharRead();
	m_bAlphaChnl = pBs->UcharRead();
	m_bRedChnl = pBs->UcharRead();
	m_bGreenChnl = pBs->UcharRead();
	m_bBlueChnl = pBs->UcharRead();
}

SFontChar::SFontChar(SBinaryStream * pBs)
{
	m_nId = pBs->UintRead();
	m_nX = pBs->UshortRead();
	m_nY = pBs->UshortRead();
	m_nWidth = pBs->UshortRead();
	m_nHeight = pBs->UshortRead();
	m_nXOffset = pBs->ShortRead();
	m_nYOffset = pBs->ShortRead();
	m_nXAdvance = pBs->ShortRead();
	m_nPage = pBs->UcharRead();
	m_nChnl = pBs->UcharRead();
}

SFontKernPair::SFontKernPair(SBinaryStream * pBs)
{
	m_nFirst = pBs->UintRead();
	m_nSecond = pBs->UintRead();
	m_nAmount = pBs->ShortRead();
}

SFont::SFont(const char * pChzBitmapfontFile) : super()
{
	m_typek = TYPEK_Font;

	// BB would be nice to have a stack allocated string class

	std::string strPath = std::string(ASSET_PATH) + pChzBitmapfontFile;

    HANDLE hFile = CreateFileA(strPath.c_str(),
							   GENERIC_READ,		   // open for reading
							   0,                      // do not share
							   NULL,                   // default security
							   OPEN_EXISTING,		   // open existing only
							   FILE_ATTRIBUTE_NORMAL,  // normal file
							   NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE) 
    { 
		MessageBoxA(0, "Font load failed", "Fatal Error", MB_OK);
		exit;
    }

	int cBytesFile = GetFileSize(hFile, nullptr);
	unsigned char * pB = new unsigned char[cBytesFile];

	if (!ReadFile(hFile, pB, cBytesFile, nullptr, nullptr))
	{
		MessageBoxA(0, "Font load failed", "Fatal Error", MB_OK);
		exit;
	}

	SBinaryStream bs(pB);
	ASSERT(bs.CharRead() == 'B' && bs.CharRead() == 'M' && bs.CharRead() == 'F' && bs.CharRead() == 3);

	SFontBlockHeader fontbhInfo = SFontBlockHeader(&bs);
	m_fontib = SFontInfoBlock(&bs);
	const char * pChzFontName = bs.PChzRead();
	m_strFontName = std::string(pChzFontName);

	SFontBlockHeader fontbhCommon = SFontBlockHeader(&bs);
	m_fontcb = SFontCommonBlock(&bs);

	SFontBlockHeader fontbhNames = SFontBlockHeader(&bs);
	for (int i = 0; i < m_fontcb.m_nPages; i++)
	{
		const char * pChzFile = bs.PChzRead();
		m_aryhTexture.push_back((new STexture(StrPrintf("fonts\\%s", pChzFile).c_str(), false, false))->HTexture());
	}

	SFontBlockHeader fontbhChars = SFontBlockHeader(&bs);
	int cFontchar = fontbhChars.m_cBytes / 20;
	for (int i = 0; i < cFontchar; i++)
	{
		m_aryFontchar.push_back(SFontChar(&bs));
	}

	// BB this header & block may be missing if no kern pairs are present

	SFontBlockHeader fontbhKern = SFontBlockHeader(&bs);
	int cFontkernpair = fontbhKern.m_cBytes / 10;
	for (int i = 0; i < cFontkernpair; i++)
	{
		m_aryFontkernpair.push_back(SFontKernPair(&bs));
	}

	delete [] pB;
}

SText::SText(SFontHandle hFont, SNodeHandle hNodeParent) : super(hNodeParent)
{
	m_typek = TYPEK_Text;

	m_hMesh = (new SMesh())->HMesh();
	m_hFont = hFont;
}

void SText::Update()
{
	int i0 = (m_nHandle + int((g_game.m_dTSyst + m_nHandle*17) * 13.0f)) % 26;
	int i1 = int(m_nHandle+ g_game.m_dTSyst * 17.0f) % 26;
	int i2 = int(m_nHandle + g_game.m_dTSyst * 19.0f) % 26;
	int i3 = int(m_nHandle + g_game.m_dTSyst * 121.0f) % 26;
	int i4 = int(m_nHandle + g_game.m_dTSyst * 123.0f) % 26;
	if (int(m_nHandle + (g_game.m_dTSyst + m_nHandle*13) * 20) % 2 == 0)
	{
		SetText(
			StrPrintf(
				"%c%c%c%c%c",
				char(i0 + 'a'),
				char(i1 + 'a'),
				char(i2 + 'a'),
				char(i3 + 'a'),
				char(i4 + 'a')));
	}
	else
	{
		SetText(
			StrPrintf(
				"%c%c%c",
				char(i0 + 'a'),
				char(i1 + 'a'),
				char(i2 + 'a')));
	}
}


void SText::SetText(const std::string & str)
{
	SFont * pFont = m_hFont.PT();

	ASSERT(pFont->m_aryhTexture.size() == 1);
	STexture * pTexture = pFont->m_aryhTexture[0].PT();

	SFontKernPair * pFontkernpair = nullptr;
	char charPrev = '\0';

	std::vector<SVertData2D> aryVertdata;
	std::vector<unsigned short> aryIIndex;
	
	float x = 0;
	for (char charCur : str)
	{
		const SFontChar * pFontchar = nullptr;
		for (int i = 0; i < pFont->m_aryFontchar.size(); i++)
		{
			const SFontChar & fontchar = pFont->m_aryFontchar[i];
			if (fontchar.m_nId == charCur)
			{
				pFontchar = &pFont->m_aryFontchar[i];
				break;
			}
		}

		ASSERT(pFontchar);

		// See https://www.angelcode.com/products/bmfont/doc/render_text.html
		//  The idea here is the "base" line is the 0 coordinate for y

		float2 vecDivisor = float2(pTexture->m_dX, pTexture->m_dY);
		float2 vecExtents = float2(pFontchar->m_nWidth, pFontchar->m_nHeight);
		float2 uvMin = float2(pFontchar->m_nX / float(pTexture->m_dX), pFontchar->m_nY / float(pTexture->m_dY));
		float2 uvMax = uvMin + vecExtents / vecDivisor;
		float2 posMax = float2(x + vecExtents.m_x, pFont->m_fontcb.m_nBase - pFontchar->m_nYOffset);
		float2 posMin = float2(x, posMax.m_y - vecExtents.m_y);
		PushQuad2D(posMin, posMax, uvMin, uvMax, &aryVertdata, &aryIIndex);

		// TODO support kerning
		x += pFontchar->m_nXAdvance;
	}

	m_hMesh->SetVerts2D(aryVertdata.data(), aryVertdata.size());
	m_hMesh->SetIndicies(aryIIndex.data(), aryIIndex.size());
}

SNode::SNode(SHandle<SNode> hNodeParent) :
	super()
{
	SetParent(hNodeParent);

	m_typek = TYPEK_Node;
}

void SNode::SetParent(SHandle<SNode> hNodeParent)
{
	if (m_hNodeParent == hNodeParent)
		return;

	// Leave previous node

	if (m_hNodeParent != -1)
	{
		if (m_hNodeSiblingPrev != -1 && m_hNodeSiblingNext != -1)
		{
			// Have previous and next sibling

			m_hNodeSiblingPrev->m_hNodeSiblingNext = m_hNodeSiblingNext;
			m_hNodeSiblingNext->m_hNodeSiblingPrev = m_hNodeSiblingPrev;
			m_hNodeSiblingPrev = -1;
			m_hNodeSiblingNext = -1;
		}
		else if (m_hNodeSiblingPrev != -1 && m_hNodeSiblingNext == -1)
		{
			// Have previous sibling only (we were the last sibling)

			m_hNodeParent->m_hNodeChildLast = m_hNodeSiblingPrev;
			m_hNodeSiblingPrev->m_hNodeSiblingNext = -1;
			m_hNodeSiblingPrev = -1;
		}
		else if (m_hNodeSiblingPrev == -1 && m_hNodeSiblingNext != -1)
		{
			// Have next sibiling only (we were the first sibling)

			m_hNodeParent->m_hNodeChildFirst = m_hNodeSiblingNext;
			m_hNodeSiblingNext->m_hNodeSiblingPrev = -1;
			m_hNodeSiblingNext = -1;
		}
		else
		{
			// Have no siblings

			m_hNodeParent->m_hNodeChildFirst = -1;
			m_hNodeParent->m_hNodeChildLast = -1;
		}
	}

	m_hNodeParent = hNodeParent;

	// Enter new node

	if (m_hNodeParent != -1)
	{
		// Append

		// BB should probably eventually support setting specific index

		if (m_hNodeParent->m_hNodeChildLast != -1)
		{
			ASSERT(m_hNodeParent->m_hNodeChildFirst != -1);

			m_hNodeParent->m_hNodeChildLast->m_hNodeSiblingNext = HNode();
			m_hNodeSiblingPrev = m_hNodeParent->m_hNodeChildLast;
			m_hNodeParent->m_hNodeChildLast = HNode();
		}
		else
		{
			m_hNodeParent->m_hNodeChildFirst = HNode();
			m_hNodeParent->m_hNodeChildLast = HNode();
		}
	}
}

SNode3D::SNode3D(SHandle<SNode> hNodeParent) :
	super(hNodeParent),
	m_transformLocal()
{
	m_typek = TYPEK_Node3D;
}

SCamera3D::SCamera3D(SHandle<SNode> hNodeParent, float radFovHorizontal, float xNearClip, float xFarClip) :
	super(hNodeParent),
	m_radFovHorizontal(radFovHorizontal),
	m_xNearClip(xNearClip),
	m_xFarClip(xFarClip)
{
	m_typek = TYPEK_Camera3D;
}

SDrawNode3D::SDrawNode3D(SHandle<SNode> hNodeParent) :
	super(hNodeParent)
{
	m_typek = TYPEK_DrawNode3D;
}

SUiNode::SUiNode(SNodeHandle hNodeParent) : super(hNodeParent)
{
	m_typek = TYPEK_UiNode;
}

void SUiNode::GetRenderConstants(SUiNodeRenderConstants * pUinoderc)
{
	pUinoderc->m_posCenter = m_pos;
	pUinoderc->m_vecScale = m_vecScale;
	pUinoderc->m_color = m_color;
}

void PushQuad2D(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<SVertData2D> * paryVertdata, std::vector<unsigned short> * paryIIndex)
{
	int iVertexStart = paryVertdata->size();
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+1);
	paryIIndex->push_back(iVertexStart+2);
	paryIIndex->push_back(iVertexStart+3);
	paryIIndex->push_back(iVertexStart+4);
	paryIIndex->push_back(iVertexStart+5);

	paryVertdata->push_back({ {posMin.m_x, posMax.m_y}, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {posMax.m_x, posMin.m_y}, {uvMax.m_x, uvMax.m_y} });
	paryVertdata->push_back({ {posMin.m_x,posMin.m_y},{uvMin.m_x,uvMax.m_y} });
	paryVertdata->push_back({ {posMin.m_x, posMax.m_y}, {uvMin.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {posMax.m_x, posMax.m_y}, {uvMax.m_x, uvMin.m_y} });
	paryVertdata->push_back({ {posMax.m_x, posMin.m_y}, {uvMax.m_x, uvMax.m_y} });
}

void PushVert(Point pos, float2 uv, std::vector<SVertData3D> * paryVertdata, int * pI)
{
	SVertData3D * vertdata = &(*paryVertdata)[(*pI)++];
	vertdata->m_pos = pos;
	vertdata->m_uv = uv;
}

void PushQuad3D(std::vector<SVertData3D> * paryVertdata, std::vector<unsigned short> * paryIIndex)
{
	int iVertexStart = paryVertdata->size();
	paryIIndex->push_back(iVertexStart);
	paryIIndex->push_back(iVertexStart+1);
	paryIIndex->push_back(iVertexStart+2);
	paryIIndex->push_back(iVertexStart+3);
	paryIIndex->push_back(iVertexStart+4);
	paryIIndex->push_back(iVertexStart+5);

	Point posLowerLeft = Point(0.0f, -1.0f, -1.0f);
	Point posLowerRight = Point(0.0f, 1.0f, -1.0f);
	Point posUpperLeft = Point(0.0f, -1.0f, 1.0f);
	Point posUpperRight = Point(0.0f, 1.0f, 1.0f);

	paryVertdata->push_back({ posUpperRight, float2(1.0f, 1.0f) });
	paryVertdata->push_back({ posLowerLeft, float2(0.0f, 0.0f) });
	paryVertdata->push_back({ posLowerRight, float2(1.0f, 0.0f) });
	paryVertdata->push_back({ posLowerLeft, float2(0.0f, 0.0f) });
	paryVertdata->push_back({ posUpperRight, float2(1.0f, 1.0f) });
	paryVertdata->push_back({ posUpperLeft, float2(0.0f, 1.0f) });
}

SMesh::SMesh()
{
	m_typek = TYPEK_Mesh;
}

// BB SetVerts3D/SetVerts2D/SetIndicies duplicate code

// BB D3D11_MAP_WRITE_DISCARD is clearing the old contents of our buffer

void SMesh::SetVerts3D(SVertData3D * aVertdata, int cVerts)
{
	// BB shouldn't immediately release this slice, should wait until we're 100% sure the gpu is done with it

	if (m_sliceVertex.m_ibStart != -1)
	{
		g_game.m_cbaVertex3D.ReleaseSlice(m_sliceVertex);
	}

	m_sliceVertex = g_game.m_cbaVertex3D.SliceClaim(cVerts * sizeof(SVertData3D));

	// BB/NOTE How would a deferred renderer handle this? If we need to update a constant buffer while the gpu theoretically might be still using it?
	//  Would we need some kind of queue?

	D3D11_MAPPED_SUBRESOURCE mappedSubresourceVerts;
	g_game.m_pD3ddevicecontext->Map(g_game.m_cbaVertex3D.m_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceVerts);
	memcpy((char *) mappedSubresourceVerts.pData + m_sliceVertex.m_ibStart, aVertdata, cVerts * sizeof(SVertData3D));
	g_game.m_pD3ddevicecontext->Unmap(g_game.m_cbaVertex3D.m_cbuffer, 0);
}

void SMesh::SetVerts2D(SVertData2D * aVertdata, int cVerts)
{
	// BB shouldn't immediately release this slice, should wait until we're 100% sure the gpu is done with it

	if (m_sliceVertex.m_ibStart != -1)
	{
		g_game.m_cbaVertex2D.ReleaseSlice(m_sliceVertex);
	}

	m_sliceVertex = g_game.m_cbaVertex2D.SliceClaim(cVerts * sizeof(SVertData2D));

	// BB/NOTE How would a deferred renderer handle this? If we need to update a constant buffer while the gpu theoretically might be still using it?
	//  Would we need some kind of queue?

	D3D11_MAPPED_SUBRESOURCE mappedSubresourceVerts;
	g_game.m_pD3ddevicecontext->Map(g_game.m_cbaVertex2D.m_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceVerts);
	memcpy((char *)mappedSubresourceVerts.pData + m_sliceVertex.m_ibStart, aVertdata, cVerts * sizeof(SVertData2D));
	g_game.m_pD3ddevicecontext->Unmap(g_game.m_cbaVertex2D.m_cbuffer, 0);
}

void SMesh::SetIndicies(unsigned short * aiIndex, int cIndex)
{
	// BB shouldn't immediately release this slice, should wait until we're 100% sure the gpu is done with it

	if (m_sliceIndex.m_ibStart != -1)
	{
		g_game.m_cbaIndex.ReleaseSlice(m_sliceIndex);
	}

	m_sliceIndex = g_game.m_cbaIndex.SliceClaim(cIndex * sizeof(unsigned short));

	// BB/NOTE How would a deferred renderer handle this? If we need to update a constant buffer while the gpu theoretically might be still using it?
	//  Would we need some kind of queue?

	D3D11_MAPPED_SUBRESOURCE mappedSubresourceIndicies;
	g_game.m_pD3ddevicecontext->Map(g_game.m_cbaIndex.m_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceIndicies);
	unsigned short * pShort = (unsigned short *) mappedSubresourceIndicies.pData;
	memcpy((char *)mappedSubresourceIndicies.pData + m_sliceIndex.m_ibStart, aiIndex, cIndex * sizeof(unsigned short));
	g_game.m_pD3ddevicecontext->Unmap(g_game.m_cbaIndex.m_cbuffer, 0);
}

void SGame::Init(HINSTANCE hInstance)
{
	AuditFixArray();
	AuditVectors();
	AuditSlotheap();

	m_hNodeRoot = (new SNode(-1))->HNode();

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

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = cVertsMax * sizeof(SVertData3D);
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		m_cbaVertex3D = SCBufferAllocator(vertexBufferDesc, sizeof(SVertData3D));
	}

	{
		const int cVertsMax = 100000;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = cVertsMax * sizeof(SVertData2D);
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		m_cbaVertex2D = SCBufferAllocator(vertexBufferDesc, sizeof(SVertData2D));
	}

	{
		const int cIndexMax = 100000;

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.ByteWidth = cIndexMax * sizeof(unsigned short);
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		m_cbaIndex = SCBufferAllocator(indexBufferDesc, sizeof(unsigned short));
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
		m_hMeshQuad = (new SMesh())->HMesh();

		std::vector<SVertData3D> aryVertdata;
		std::vector<unsigned short> aryIIndex;

		PushQuad3D(&aryVertdata, &aryIIndex);

		m_hMeshQuad->SetVerts3D(aryVertdata.data(), aryVertdata.size());
		m_hMeshQuad->SetIndicies(aryIIndex.data(), aryIIndex.size());
	}

	SShaderHandle hShaderUnlit = (new SShader(L"shaders\\unlit2d.hlsl", false))->HShader();
	SShaderHandle hShaderText = (new SShader(L"shaders\\text2d.hlsl", false))->HShader();
	SShaderHandle hShaderLit = (new SShader(L"shaders\\lit2d.hlsl", false))->HShader();

	SShaderHandle hShader3D = (new SShader(L"shaders\\unlit3d.hlsl", true))->HShader();

	// Font 

	m_hFont = (new SFont("fonts\\candara.fnt"))->HFont();

	SMaterialHandle hMaterialText = (new SMaterial(hShaderText))->HMaterial();
	hMaterialText->m_hTexture = m_hFont->m_aryhTexture[0];

	m_hText = (new SText(m_hFont, m_hNodeRoot))->HText();
	m_hText->m_hMaterial = hMaterialText;
	m_hText->SetText("Score: joy");
	m_hText->m_vecScale = float2(1.0f, 1.0f);
	m_hText->m_gSort = 10.0f;
	m_hText->m_pos = float2(0.0f, 0.0f);
	m_hText->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	STextHandle hText2 = (new SText(m_hFont, m_hNodeRoot))->HText();
	hText2->m_hMaterial = hMaterialText;
	hText2->SetText("Score: joy");
	hText2->m_vecScale = float2(1.0f, 1.0f);
	hText2->m_gSort = 10.0f;
	hText2->m_pos = float2(50.0f, 50.0f);
	hText2->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Camera

	m_hCamera3D = (new SCamera3D(m_hNodeRoot, RadFromDeg(90.0f), 0.1, 100.0f))->HCamera3D();

	m_hPlaneTest = (new SDrawNode3D(m_hNodeRoot))->HDrawnode3D();
	m_hPlaneTest->m_hMaterial = (new SMaterial(hShader3D))->HMaterial();
	m_hPlaneTest->m_transformLocal.m_pos = Point(10.0f, 0.0f, 0.0f);
	m_hPlaneTest->m_hMesh = m_hMeshQuad;

	SDrawNode3DHandle hPlaneTest2 = (new SDrawNode3D(m_hNodeRoot))->HDrawnode3D();
	hPlaneTest2->m_hMaterial = (new SMaterial(hShader3D))->HMaterial();
	hPlaneTest2->m_transformLocal.m_pos = Point(10.0f, 1.0f, 0.0f);
	hPlaneTest2->m_hMesh = m_hMeshQuad;
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
			double m_dTSystDoublePrev = m_dTSystDouble;
			LARGE_INTEGER perfCount;
			QueryPerformanceCounter(&perfCount);

			m_dTSystDouble = (double) (perfCount.QuadPart - m_startPerfCount) / (double) m_perfCounterFrequency;
			m_dT = (float) (m_dTSystDouble - m_dTSystDoublePrev);
			if (m_dT > (1.f / 60.f))
				m_dT = (1.f / 60.f);
		}
		m_dTSyst = float(m_dTSystDouble);

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
		m_hPlaneTest->m_transformLocal.m_quat = QuatAxisAngle(g_vecZAxis, m_dT) * m_hPlaneTest->m_transformLocal.m_quat;
		//m_hPlaneTest->m_transformLocal.m_quat = QuatAxisAngle(g_vecZAxis, *m_dT) * m_hPlaneTest->m_transformLocal.m_quat;

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

		std::vector<SUiNodeHandle> aryhUinodeToRender;
		std::vector<SDrawNode3DHandle> aryhDrawnode3DToRender;

		// BB I think eventually we'd want render all draw nodes once per camera

		SCamera3DHandle hCamera3D = -1;

		for (SNodeHandle hNode : aryhNode)
		{
			if (hNode != -1)
			{
				hNode->Update();
				if (hNode->FIsDerivedFrom(TYPEK_UiNode))
				{
					aryhUinodeToRender.push_back(SUiNodeHandle(hNode.m_id));
				}
				else if (hNode->FIsDerivedFrom(TYPEK_Camera3D))
				{
					ASSERT(hCamera3D == -1);
					hCamera3D = SCamera3DHandle(hNode.m_id);
				}
				else if (hNode->FIsDerivedFrom(TYPEK_DrawNode3D))
				{
					aryhDrawnode3DToRender.push_back(SDrawNode3DHandle(hNode.m_id));
				}
			}
		}

		// Sort UI nodes

		std::qsort(aryhUinodeToRender.data(), aryhUinodeToRender.size(), sizeof(SUiNodeHandle), SortUinodeRenderOrder);

		// Start rendering stuff

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

		// Draw 3d nodes

		{
			m_pD3ddevicecontext->RSSetState(m_pD3drasterizerstate);
			m_pD3ddevicecontext->OMSetDepthStencilState(m_pD3ddepthstencilstate, 0);

			m_pD3ddevicecontext->OMSetRenderTargets(1, &m_pD3dframebufferview, m_pD3ddepthstencilview);

			SCamera3D * pCamera3D = hCamera3D.PT();

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

				const SMesh & mesh = *hDrawnode3D->m_hMesh;

				m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);

				m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
				m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

				ID3D11Buffer * aD3dbuffer[] = { m_cbufferDrawnode3D, m_cbufferGlobals };
				m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
				m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

				unsigned int cbVert = sizeof(SVertData3D);
				m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbaVertex3D.m_cbuffer, &cbVert, &g_cbMeshOffset);		// BB don't constantly do this
				m_pD3ddevicecontext->IASetIndexBuffer(m_cbaIndex.m_cbuffer, DXGI_FORMAT_R16_UINT, 0);					//  ...

				D3D11_MAPPED_SUBRESOURCE mappedSubresource;
				m_pD3ddevicecontext->Map(m_cbufferDrawnode3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
				SDrawNodeRenderConstants * pDrawnode3Drc = (SDrawNodeRenderConstants *) (mappedSubresource.pData);

				// TODO don't constantly recompute

				Mat matModel = hDrawnode3D->m_transformLocal.Mat();
				Mat matCamera = hCamera3D->m_transformLocal.Mat().MatInverse();
				Mat matPerspective = MatPerspective(hCamera3D->m_radFovHorizontal, vecWinSize.m_x / vecWinSize.m_y, hCamera3D->m_xNearClip, hCamera3D->m_xFarClip);
				ASSERT(sizeof(Mat) == sizeof(float) * 16);
				pDrawnode3Drc->m_matMVP = matModel * matCamera * matPerspective;

				m_pD3ddevicecontext->Unmap(m_cbufferDrawnode3D, 0);

				//m_pD3ddevicecontext->Draw(mesh.m_sliceVertex.m_cb / sizeof(SVertData3D), mesh.m_sliceVertex.m_ibStart / sizeof(SVertData3D));
				m_pD3ddevicecontext->DrawIndexed(mesh.m_sliceIndex.m_cb / sizeof(unsigned int), mesh.m_sliceIndex.m_ibStart / sizeof(unsigned int), mesh.m_sliceVertex.m_ibStart / sizeof(SVertData3D));
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
			switch (hUinode->m_typek)
			{
				case TYPEK_Text:
					{
						const SMesh & mesh = *hUinode->m_hMesh;
						const STexture & texture = *material.m_hTexture;

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(m_pD3dblendstatenoblend, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferUiNode, m_cbufferGlobals };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

						unsigned int cbVert = sizeof(SVertData2D);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbaVertex2D.m_cbuffer, &cbVert, &g_cbMeshOffset);		// BB don't constantly do this

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
						hUinode->GetRenderConstants(pUinoderc);
						m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_sliceVertex.m_cb / sizeof(SVertData2D), mesh.m_sliceVertex.m_ibStart / sizeof(SVertData2D));
					}
					break;
				default:
					{
						const SMesh & mesh = *hUinode->m_hMesh;
						const STexture & texture = *material.m_hTexture;

						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(nullptr, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferUiNode, m_cbufferGlobals };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);

						unsigned int cbVert = sizeof(SVertData2D);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_cbaVertex2D.m_cbuffer, &cbVert, &g_cbMeshOffset);		// BB don't constantly do this

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
						hUinode->GetRenderConstants(pUinoderc);
						m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_sliceVertex.m_cb / sizeof(SVertData2D), mesh.m_sliceVertex.m_ibStart / sizeof(SVertData2D));
					}
					break;
			}
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
