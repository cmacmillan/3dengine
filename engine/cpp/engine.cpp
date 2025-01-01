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

SShader::SShader(LPCWSTR lpcwstrFilename) : super()
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


void SText::SetText(const std::string & str)
{
	SFont * pFont = m_hFont.PT();

	ASSERT(pFont->m_aryhTexture.size() == 1);
	STexture * pTexture = pFont->m_aryhTexture[0].PT();

	SFontKernPair * pFontkernpair = nullptr;
	char charPrev = '\0';

	std::vector<float> aryVerts;
	
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
		PushQuad(posMin, posMax, uvMin, uvMax, &aryVerts);

		// TODO support kerning
		x += pFontchar->m_nXAdvance;
	}


	int cStride = 4 * sizeof(float);
	int cVerts = aryVerts.size() / 4;
	m_hMesh->SetVerts(aryVerts.data(), cVerts, cStride, 0);
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

// BB This Text Update is good to induce the text memory issue, revisit

/*
int blah = 0;
float delta = 0;
void SText::Update()
{
	return;
	delta += g_game.m_dT;
	if (delta > 2.0f)
	{
		delta = 0.0f;
		blah++;
		blah = blah % 26;
		char aCh[] = {'a', '\0'};
		aCh[0] = 'a' + blah;
		SetText((const char *)aCh);
	}
}
*/

void PushQuad(float2 posMin, float2 posMax, float2 uvMin, float2 uvMax, std::vector<float> * pAryVert)
{
	int iQuadStart = pAryVert->size();

	// BB not reusing verts?
	//  Where is our index buffer lol

	pAryVert->resize(pAryVert->size() + 6 * 4);

	{
		(*pAryVert)[iQuadStart] = posMin.m_x;
		(*pAryVert)[iQuadStart + 1] = posMax.m_y;
		(*pAryVert)[iQuadStart + 2] = uvMin.m_x;
		(*pAryVert)[iQuadStart + 3] = uvMin.m_y;
	}

	{
		(*pAryVert)[iQuadStart + 4] = posMax.m_x;
		(*pAryVert)[iQuadStart + 5] = posMin.m_y;
		(*pAryVert)[iQuadStart + 6] = uvMax.m_x;
		(*pAryVert)[iQuadStart + 7] = uvMax.m_y;
	}

	{
		(*pAryVert)[iQuadStart + 8] = posMin.m_x;
		(*pAryVert)[iQuadStart + 9] = posMin.m_y;
		(*pAryVert)[iQuadStart + 10] = uvMin.m_x;
		(*pAryVert)[iQuadStart + 11] = uvMax.m_y;
	}

	{
		(*pAryVert)[iQuadStart + 12] = posMin.m_x;
		(*pAryVert)[iQuadStart + 13] = posMax.m_y;
		(*pAryVert)[iQuadStart + 14] = uvMin.m_x;
		(*pAryVert)[iQuadStart + 15] = uvMin.m_y;
	}

	{
		(*pAryVert)[iQuadStart + 16] = posMax.m_x;
		(*pAryVert)[iQuadStart + 17] = posMax.m_y;
		(*pAryVert)[iQuadStart + 18] = uvMax.m_x;
		(*pAryVert)[iQuadStart + 19] = uvMin.m_y;
	}

	{
		(*pAryVert)[iQuadStart + 20] = posMax.m_x;
		(*pAryVert)[iQuadStart + 21] = posMin.m_y;
		(*pAryVert)[iQuadStart + 22] = uvMax.m_x;
		(*pAryVert)[iQuadStart + 23] = uvMax.m_y;
	}
}

SMesh::SMesh()
{
	m_typek = TYPEK_Mesh;
}

void SMesh::SetVerts(float * pGVerts, int cVerts, int cStride, int cOffset)
{
	// Destroy old buffer

	// BB would really be better to hand out pieces of a larger buffer e.g. https://www.gamedev.net/forums/topic/574484-d3d11-how-when-to-properly-destroy-a-vertex-buffer/4664871/
	//  That discussion also mentions that releasing the buffer like this can cause artifacts if the buffer is still being used for rendering
	//  Probably not a problem for my highly sequential renderer?

	if (m_cbufferVertex != nullptr)
	{
		m_cbufferVertex->Release();
		m_cbufferVertex = nullptr;
	}

	m_cVerts = cVerts;
	m_cStride = cStride;
	m_cOffset = cOffset;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = cVerts * cStride * sizeof(float);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresourceData = { pGVerts };

	HRESULT hResult = g_game.m_pD3ddevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_cbufferVertex);
	assert(SUCCEEDED(hResult));
}

void SGame::Init(HINSTANCE hInstance)
{
	AuditFixArray();
	AuditVectors();

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

	// Create quad mesh

	{
		m_hMeshQuad = (new SMesh())->HMesh();

		std::vector<float> aryVert;

		PushQuad(float2(-0.5f, -0.5f), float2(0.5f, 0.5f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), &aryVert);

		int cStride = 4 * sizeof(float);
		int cVerts = aryVert.size() / 4;
		m_hMeshQuad->SetVerts(aryVert.data(), cVerts, cStride, 0);
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
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
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

	SShaderHandle hShaderUnlit = (new SShader(L"shaders\\unlit2d.hlsl"))->HShader();
	SShaderHandle hShaderText = (new SShader(L"shaders\\text2d.hlsl"))->HShader();
	SShaderHandle hShaderLit = (new SShader(L"shaders\\lit2d.hlsl"))->HShader();

	SShaderHandle hShader3D = (new SShader(L"shaders\\lit2d.hlsl"))->HShader();

	// Font 

	m_hFont = (new SFont("fonts\\candara.fnt"))->HFont();

	m_hText = (new SText(m_hFont, m_hNodeRoot))->HText();
	m_hText->m_hMaterial = (new SMaterial(hShaderText))->HMaterial();
	m_hText->m_hMaterial->m_hTexture = m_hText->m_hFont->m_aryhTexture[0];
	m_hText->SetText("Score: joy");
	m_hText->m_vecScale = float2(1.0f, 1.0f);
	m_hText->m_gSort = 10.0f;
	m_hText->m_pos = float2(0.0f, 0.0f);
	m_hText->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Camera

	m_hCamera3D = (new SCamera3D(m_hNodeRoot, RadFromDeg(90.0f), 0.1, 100.0f))->HCamera3D();

	m_hPlaneTest = (new SDrawNode3D(m_hNodeRoot))->HDrawnode3D();
	m_hPlaneTest->m_hMaterial = (new SMaterial(hShader3D))->HMaterial();
	m_hPlaneTest->m_hMesh = m_hMeshQuad;
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
	
		//m_hText->m_vecScale = GSin(m_dTSyst) * float2(1.0f, 1.0f);

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

			for (SDrawNode3DHandle hDrawnode3D : aryhDrawnode3DToRender)
			{
				if (!hDrawnode3D.PT())
					continue;

				if (hDrawnode3D->m_hMaterial == nullptr)
					continue;

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
				m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);

				D3D11_MAPPED_SUBRESOURCE mappedSubresource;
				m_pD3ddevicecontext->Map(m_cbufferDrawnode3D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
				SDrawNodeRenderConstants * pDrawnode3Drc = (SDrawNodeRenderConstants *) (mappedSubresource.pData);
				pDrawnode3Drc->m_matMVP = hDrawnode3D->m_transformLocal.Mat() * hCamera3D->m_transformLocal.Mat().MatInverse() * MatPerspective(hCamera3D->m_radFovHorizontal, vecWinSize.m_x / vecWinSize.m_y, hCamera3D->m_xNearClip, hCamera3D->m_xFarClip); // TODO don't constantly recompute
				m_pD3ddevicecontext->Unmap(m_cbufferDrawnode3D, 0);

				m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);
				//m_pD3ddevicecontext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

				m_pD3ddevicecontext->Draw(mesh.m_cVerts, 0);
				//m_pD3ddevicecontext->DrawIndexed(numIndices, 0, 0);
			}
		}

#if 0
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
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
						hUinode->GetRenderConstants(pUinoderc);
						m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_cVerts, 0);
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
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferUiNode, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SUiNodeRenderConstants * pUinoderc = (SUiNodeRenderConstants *) (mappedSubresource.pData);
						hUinode->GetRenderConstants(pUinoderc);
						m_pD3ddevicecontext->Unmap(m_cbufferUiNode, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_cVerts, 0);
					}
					break;
			}
		}
#endif

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
