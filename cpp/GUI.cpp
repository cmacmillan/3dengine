// Uses some d3d api code from https://github.com/kevinmoran/BeginnerDirect3D11

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gui.h"
#include "scrabblesolver.h"
#include "boardlayouts.h"

#include "scrabblesolver.h"

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

float2 SGame::PosCursorWorld()
{
	return m_hCamera->PosWorldFromCursor({ float(m_xCursor), float(m_yCursor) });
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

float2 float2::operator/(float g) const
{
	return float2(m_x / g, m_y / g);
}

float2 float2::operator*(float g) const
{
	return float2(m_x * g, m_y * g);
}

float2 float2::operator/(float2 vec) const
{
	return float2(m_x / vec.m_x, m_y / vec.m_y);
}

float2 float2::operator*(float2 vec) const
{
	return float2(m_x * vec.m_x, m_y * vec.m_y);
}

float2 float2::operator+(float2 vec) const
{
	return float2(m_x + vec.m_x, m_y + vec.m_y);
}

float2 float2::operator-(float2 vec) const
{
	return float2(m_x - vec.m_x, m_y - vec.m_y);
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

	std::string strPath = std::string(ASSET_PATH) + pChzFilename;

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

	std::wstring wstrPath = wstrFromStr(std::string(ASSET_PATH)) + std::wstring(lpcwstrFilename);

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

SGameObject::SGameObject() : super()
{
	m_typek = TYPEK_GameObject;

	g_game.m_aryhGo.push_back(HGo());
}

SScrabbleTile::SScrabbleTile() : super()
{
	m_typek = TYPEK_ScrabbleTile;

	g_game.m_aryhTile.push_back(HTile());
}

void SScrabbleTile::Update()
{
	int iRack = IFind(g_game.m_hGridBoard->m_aryhTileRack, HTile());

	if (iRack != -1)
	{
		int cRack = g_game.m_hGridBoard->m_aryhTileRack.size();
		float xOffset = (iRack / float(cRack) - 0.5f) * VEC_RACK_GAP * cRack;
		m_posTarget = POS_RACK_CENTER + float2(xOffset, 0.0f);
	}

	m_pos = lerp(m_pos, m_posTarget, 20.0f * g_game.m_dT);

	if (g_game.m_hGridBoard->m_hTileSelected == HTile())
	{
		if (g_game.m_hGridBoard->m_fPicked)
		{
			m_vecScale = PIECE_SCALE_PICKED;
		}
		else
		{
			m_vecScale = PIECE_SCALE_HOVERED;
		}

		m_gSort = SORT_TILE_PICKED;
	}
	else
	{
		m_vecScale = PIECE_SCALE;
		m_gSort = SORT_TILE_NORMAL;
	}
}

SCamera::SCamera() : super()
{
	m_typek = TYPEK_Camera;
}

SScrabbleGrid::SScrabbleGrid() : super()
{
	m_typek = TYPEK_ScrabbleGrid;

	for (int i = 0; i < DIM(m_ahTileGrid); i++)
	{
		m_ahTileGrid[i] = -1;
	}
}

void SScrabbleGrid::Update()
{
#if TESTING_WORD_SCORES
	if (g_game.m_iMoveWordscore == -1)
		return;

	int iMoveWordscoreNext = g_game.m_iMoveWordscore;
	if (g_game.m_sScroll > 0.0f)
	{
		iMoveWordscoreNext--;
		if (iMoveWordscoreNext < 0)
			iMoveWordscoreNext = g_game.m_aryMoveWordscore.size() - 1;
	}
	else if (g_game.m_sScroll < 0.0f)
	{
		iMoveWordscoreNext++;
		if (iMoveWordscoreNext >= g_game.m_aryMoveWordscore.size())
			iMoveWordscoreNext = 0;
	}
	g_game.SetShowMovescore(iMoveWordscoreNext);

#else
	// Spawn new tiles

	for (int vk = VK_A; vk <= VK_Z; vk++)
	{
		if (g_game.m_mpVkFJustPressed[vk])
		{
			char ch = 'a' + vk - VK_A;
			g_game.m_hGridBoard->m_aryhTileRack.push_back(HTileSpawn(ch));
		}
	}

	float2 posCursorWorld = g_game.PosCursorWorld();
	if (m_fPicked)
	{
		m_hTileSelected->m_posTarget = posCursorWorld + m_vecPickOffset;
		if (!g_game.m_mpVkFDown[VK_LBUTTON])
		{
			float2 vecIntGrid = VecIntGridCoordsFromWorld(m_hTileSelected->m_posTarget);
			int x = int(vecIntGrid.m_x);
			int y = int(vecIntGrid.m_y);
			if (x >= 0 && y >= 0 && x < DX_GRID && y < DY_GRID)
			{
				AddTileToGrid(m_hTileSelected, x, y);
			}
			else
			{
				m_aryhTileRack.push_back(m_hTileSelected);
			}
			m_fPicked = false;
		}
	}
	else
	{
		float2 vecScale = PIECE_SCALE;
		vecScale = vecScale / 2.0f;
		SScrabbleTileHandle hTileBest = -1;
		for (SScrabbleTileHandle hTile : g_game.m_aryhTile)
		{
			float2 posMin = hTile->m_pos - vecScale;
			float2 posMax = hTile->m_pos + vecScale;
			bool fHovered = posCursorWorld.m_x > posMin.m_x && posCursorWorld.m_y > posMin.m_y && posCursorWorld.m_x < posMax.m_x && posCursorWorld.m_y < posMax.m_y;
			if (fHovered && (!hTileBest || hTileBest->m_gSort<hTile->m_gSort))
			{
				hTileBest = hTile;
			}
		}

		m_hTileSelected = hTileBest;

		if (g_game.m_mpVkFJustPressed[VK_LBUTTON] && m_hTileSelected)
		{
			if (m_hTileSelected->m_iGrid != -1)
			{
				m_ahTileGrid[m_hTileSelected->m_iGrid] = -1;
				m_hTileSelected->m_iGrid = -1;
			}
			int iRack = IFind(m_aryhTileRack, m_hTileSelected);
			if (iRack != -1)
			{
				m_aryhTileRack.erase(m_aryhTileRack.begin() + iRack);
			}
			m_fPicked = true;
			m_vecPickOffset = m_hTileSelected->m_pos - posCursorWorld;
		}
	}
#endif
}

void SScrabbleGrid::CopyGameStateFromPchz(const char * pChzBoardLayout, const char * pChzRack)
{
	// TODO clear rack & board

    ASSERT(strlen(pChzBoardLayout) == DX_GRID * DY_GRID);
    for (int x = 0; x < DX_GRID; x++)
    {
        for (int y = 0; y < DY_GRID; y++)
        {
            // Reversing y so 0, 0 is the bottom left corner
            int i = IChFromCoords(x, (DY_GRID - 1) - y);
            char ch = pChzBoardLayout[i];
            if (ch != '0')
			{
                ch = ToLower(ch);
				AddTileToGrid(HTileSpawn(ch), x, y);
			}
        }
    }

	int cRack = strlen(pChzRack);
	for (int iRack = 0; iRack < cRack; iRack++)
	{
		m_aryhTileRack.push_back(HTileSpawn(pChzRack[iRack]));
	}
}

float2 SScrabbleGrid::PosLowerLeft()
{
	return m_pos - float2(1.0f, 1.0f) * CELL_SIZE * DX_GRID / 2.0f;
}

SScrabbleTileHandle SScrabbleGrid::HTileSpawn(char ch)
{
	SScrabbleTile * pTile = new SScrabbleTile();
	pTile->m_chLetter = ToLower(ch);
	pTile->m_hMaterial = g_game.m_hMaterialTile;
	pTile->m_hMesh = g_game.m_hMeshQuad;
	pTile->m_vecScale = PIECE_SCALE;
	pTile->m_gSort = SORT_TILE_NORMAL;
	return pTile->HTile();
}

void SScrabbleGrid::AddTileToGrid(SScrabbleTileHandle hTile, int x, int y)
{
	hTile->m_posTarget = PosLowerLeft() + PIECE_SCALE / 2.0f + float2(x, y) * CELL_SIZE;
	int iGrid = IChFromCoords(x, y);
	if (m_ahTileGrid[iGrid])
	{
		m_aryhTileRack.push_back(hTile);
	}
	else
	{
		m_ahTileGrid[iGrid] = hTile;
		hTile->m_iGrid = iGrid;
	}
}

float2 SScrabbleGrid::VecIntGridCoordsFromWorld(float2 posWorld)
{
	float2 gridLowerLeft = PosLowerLeft();
	float2 vecGridOffset = (posWorld - gridLowerLeft)/CELL_SIZE;
	int x = floor(vecGridOffset.m_x);
	int y = floor(vecGridOffset.m_y);
	return float2(x, y);
}

void SCamera::Update()
{
	float2 vecWinSize = g_game.VecWinSize();
	float gScaleCamera = (BOARD_SCALE + BOARD_PADDING) * 0.5f / min(vecWinSize.m_x, vecWinSize.m_y);
	m_vecExtents = { gScaleCamera * vecWinSize.m_x, gScaleCamera * vecWinSize.m_y };
}

float2 SCamera::PosWorldFromCursor(float2 posCursor)
{
	float2 posBottomLeft = m_pos - m_vecExtents;
	float2 posTopRight = m_pos + m_vecExtents;
	float2 vecWinSize = g_game.VecWinSize();
	return float2(
		maprange(0.0f, vecWinSize.m_x, posBottomLeft.m_x, posTopRight.m_x, posCursor.m_x),
		maprange(vecWinSize.m_y, 0.0f, posBottomLeft.m_y, posTopRight.m_y, posCursor.m_y)); // NOTE inverted y
}

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
		m_aryhTexture.push_back((new STexture(pChzFile, false, false))->HTexture());
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

SText::SText(SFontHandle hFont)
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

		float2 vecDivisor = float2(pTexture->m_dX, pTexture->m_dY);
		float2 vecExtents = float2(pFontchar->m_nWidth, pFontchar->m_nHeight);
		float2 uvMin = float2(pFontchar->m_nX / float(pTexture->m_dX), pFontchar->m_nY / float(pTexture->m_dY));
		float2 uvMax = uvMin + vecExtents / vecDivisor;
		//float2 posMin = float2(x, 0.0f) - float2(pFontchar->m_nXOffset, pFontchar->m_nYOffset);
		float2 posMin = float2(x, -pFontchar->m_nYOffset-vecExtents.m_y);//+ float2(0.0f, pFontchar->m_nYOffset);
		float2 posMax = float2(x + vecExtents.m_x, -pFontchar->m_nYOffset);
		//float2 posMax = posMin + vecExtents;
		PushQuad(posMin, posMax, uvMin, uvMax, &aryVerts);

		// TODO support kerning
		// TODO use nXAdvance?
		x += vecExtents.m_x;
	}


	int cStride = 4 * sizeof(float);
	int cVerts = aryVerts.size() / 4;
	m_hMesh->SetVerts(aryVerts.data(), cVerts, cStride, 0);
}

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
			L"Scrabble",
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
		ID3D11Texture2D * d3d11FrameBuffer;
		HRESULT hResult = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
		assert(SUCCEEDED(hResult));

		hResult = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, 0, &m_pD3dframebufferview);
		assert(SUCCEEDED(hResult));
		d3d11FrameBuffer->Release();
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
		D3D11_BUFFER_DESC descCbufferObject = {};
		CASSERT(sizeof(SGameObjectRenderConstants) % 16 == 0);
		descCbufferObject.ByteWidth = sizeof(SGameObjectRenderConstants);
		descCbufferObject.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferObject.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferObject.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferObject, nullptr, &m_cbufferObject);
		assert(SUCCEEDED(hResult));
	}

	{
		D3D11_BUFFER_DESC descCbufferCamera = {};
		CASSERT(sizeof(ShaderGlobals) % 16 == 0);
		descCbufferCamera.ByteWidth = sizeof(ShaderGlobals);
		descCbufferCamera.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferCamera.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferCamera.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferCamera, nullptr, &m_cbufferGlobals);
		assert(SUCCEEDED(hResult));
	}

	{
		D3D11_BUFFER_DESC descCbufferScrabbleTileConstants = {};
		CASSERT(sizeof(SScrabbleTileConstants) % 16 == 0);
		descCbufferScrabbleTileConstants.ByteWidth = sizeof(SScrabbleTileConstants);
		descCbufferScrabbleTileConstants.Usage = D3D11_USAGE_DYNAMIC;
		descCbufferScrabbleTileConstants.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descCbufferScrabbleTileConstants.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = m_pD3ddevice->CreateBuffer(&descCbufferScrabbleTileConstants, nullptr, &m_cbufferScrabbleTile);
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

	m_pSolver = new SScrabbleSolver("scrabbleword.dat");

	SShaderHandle hShaderUnlit = (new SShader(L"unlit2d.hlsl"))->HShader();
	SShaderHandle hShaderText = (new SShader(L"text.hlsl"))->HShader();
	SShaderHandle hShaderLit = (new SShader(L"lit2d.hlsl"))->HShader();

	// Font 

	m_hFont = (new SFont("candara.fnt"))->HFont();

	m_hText = (new SText(m_hFont))->HText();
	m_hText->m_hMaterial = (new SMaterial(hShaderText))->HMaterial();
	m_hText->m_hMaterial->m_hTexture = m_hText->m_hFont->m_aryhTexture[0];
	m_hText->SetText("Score:");
	//m_hText->m_vecScale = float2(0.2f, 0.2f);
	m_hText->m_vecScale = float2(0.025f, 0.025f);
	m_hText->m_gSort = 10.0f;
	m_hText->m_pos = float2(0.0f, BOARD_SCALE * 0.5f + 6.0f);
	m_hText->m_color = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Camera 

	m_hCamera = (new SCamera())->HCamera();

	// Board

	m_hGridBoard = (new SScrabbleGrid())->HGrid();

	m_hGridBoard->m_hMaterial = (new SMaterial(hShaderLit))->HMaterial();

	m_hGridBoard->m_hMaterial->m_hTexture = (new STexture("BoardAlbedoWithText.png", false, true))->HTexture();
	//m_hGridBoard->m_hMaterial->m_hTexture2 = (new STexture("BoardNormalWithoutText_srgb.png", true))->HTexture();
	m_hGridBoard->m_hMaterial->m_hTexture2 = (new STexture("webboardnormal.png", true, false))->HTexture();
	m_hGridBoard->m_pos = float2(0.0f, 0.0f);
	m_hGridBoard->m_vecScale = float2(BOARD_SCALE, BOARD_SCALE);
	m_hGridBoard->m_hMesh = m_hMeshQuad;
	//m_hGridBoard->m_vecScale = { .001f, .001f};
	m_hGridBoard->m_gSort = 0.0f;

	// Piece

	SShaderHandle hShaderTile = (new SShader(L"scrabbletile.hlsl"))->HShader();

	m_hMaterialTile = (new SMaterial(hShaderTile))->HMaterial();
	m_hMaterialTile->m_hTexture = (new STexture("TileAlbedo.png", false, true))->HTexture();
	m_hMaterialTile->m_hTexture2 = (new STexture("TileNormal.png", true, false))->HTexture();


#if TESTING_WORD_SCORES
	SSolverBoard board(g_pChzBoardTacoOats);
	SRack rack(g_pChzRackTacoOats);
	m_hGridBoard->CopyGameStateFromPchz(g_pChzBoardTacoOats, g_pChzRackTacoOats);
	m_pSolver->FindValidMoves(board, rack, &m_aryMoveWordscore);
	if (m_aryMoveWordscore.size() > 0)
	{
		SetShowMovescore(0);
	}
#endif
}

int SortGameObjectHandles(const void * pVa, const void * pVb)
{
	SGameObjectHandle hGoA = *(SGameObjectHandle*) pVa;
	SGameObjectHandle hGoB = *(SGameObjectHandle*) pVb;
	if (hGoA->m_gSort == hGoB->m_gSort)
		return 0;
	else if (hGoA->m_gSort < hGoB->m_gSort)
		return -1;
	else
		return 1;
}

void SGame::MainLoop()
{
	ID3D11BlendState1* g_pBlendStateNoBlend = NULL;
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

	m_pD3ddevice->CreateBlendState1(&BlendState, &g_pBlendStateNoBlend);

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

			HRESULT res = m_pD3dswapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			assert(SUCCEEDED(res));

			ID3D11Texture2D * d3d11FrameBuffer;
			res = m_pD3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &d3d11FrameBuffer);
			assert(SUCCEEDED(res));

			res = m_pD3ddevice->CreateRenderTargetView(d3d11FrameBuffer, NULL,
				&m_pD3dframebufferview);
			assert(SUCCEEDED(res));
			d3d11FrameBuffer->Release();

			m_fDidWindowResize = false;
		}

		float2 vecWinSize = VecWinSize();

		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, vecWinSize.m_x, vecWinSize.m_y, 0.0f, 1.0f };

		//////////////////////// GAMEPLAY CODE

		SSolverBoard solverboard = SSolverBoard(m_hGridBoard);
#if TESTING_WORD_SCORES
		m_hText->SetText(std::to_string(m_iMoveWordscore));
#endif

		if (m_mpVkFDown[VK_ESCAPE])
			DestroyWindow(m_hwnd);

		////////////////////////

		// Sort

		// BB Copying so newly spawned objects won't try to update. However we probably do want them to update & render the frame they spawn. Think through more

		std::vector<SGameObjectHandle> aryhGoCopy = m_aryhGo;

		std::qsort(aryhGoCopy.data(), aryhGoCopy.size(), sizeof(SGameObjectHandle), SortGameObjectHandles);

		{
			D3D11_MAPPED_SUBRESOURCE mappedSubresourceCamera;
			m_pD3ddevicecontext->Map(m_cbufferGlobals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresourceCamera);
			ShaderGlobals * pShaderglobals = (ShaderGlobals *) (mappedSubresourceCamera.pData);
			pShaderglobals->m_posCameraCenter = m_hCamera->m_pos;
			pShaderglobals->m_vecCameraSize = m_hCamera->m_vecExtents;
			pShaderglobals->m_t = m_dTSyst;
			m_pD3ddevicecontext->Unmap(m_cbufferGlobals, 0);
		}

		FLOAT backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_pD3ddevicecontext->ClearRenderTargetView(m_pD3dframebufferview, backgroundColor);
		m_pD3ddevicecontext->RSSetViewports(1, &viewport);
		m_pD3ddevicecontext->OMSetRenderTargets(1, &m_pD3dframebufferview, nullptr);


		for (SGameObjectHandle hGo : aryhGoCopy)
		{
			if (!hGo.PT())
				continue;

			// BB gross to tie update order to render order
			hGo->Update();
			if (hGo->m_hMaterial == nullptr)
				continue;
			const SMaterial & material = *hGo->m_hMaterial;
			const SShader & shader = *(material.m_hShader);
			switch (hGo->m_typek)
			{
				case TYPEK_ScrabbleTile:
					{
						SScrabbleTile * pTile = static_cast<SScrabbleTile *>(hGo.PT());
						const STexture & textureAlbedo = *material.m_hTexture;
						const STexture & textureNormal = *material.m_hTexture2;

						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(nullptr, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferObject, m_cbufferGlobals, m_cbufferScrabbleTile };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_hMeshQuad->m_cbufferVertex, &m_hMeshQuad->m_cStride, &m_hMeshQuad->m_cOffset);

						{
							D3D11_MAPPED_SUBRESOURCE mappedSubresource;
							m_pD3ddevicecontext->Map(m_cbufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
							SGameObjectRenderConstants * pGorc = (SGameObjectRenderConstants *) (mappedSubresource.pData);
							pGorc->m_posCenter = hGo->m_pos;
							pGorc->m_vecScale = hGo->m_vecScale;
							pGorc->m_color = hGo->m_color;
							m_pD3ddevicecontext->Unmap(m_cbufferObject, 0);
						}

						{
							// BB could stuff all this constant into a single buffer & just use the subresource index parameter on map/unmap?

							D3D11_MAPPED_SUBRESOURCE mappedSubresource;
							m_pD3ddevicecontext->Map(m_cbufferScrabbleTile, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
							SScrabbleTileConstants * pStc = (SScrabbleTileConstants *) (mappedSubresource.pData);
							int iOffset = pTile->m_chLetter - 'a';
							const int dX = 8;
							int y = iOffset / dX;
							int x = iOffset - y * dX;
							pStc->m_uvTopLeft = { x / float(dX), y / float(dX) };
							m_pD3ddevicecontext->Unmap(m_cbufferScrabbleTile, 0);
						}

						ID3D11ShaderResourceView * aD3dsrview[] = { textureAlbedo.m_pD3dsrview, textureNormal.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { textureAlbedo.m_pD3dsamplerstate, textureNormal.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(m_hMeshQuad->m_cVerts, 0);
					}
					break;

				case TYPEK_ScrabbleGrid:
					{
						SScrabbleTile * pTile = static_cast<SScrabbleTile *>(hGo.PT());
						const STexture & textureAlbedo = *material.m_hTexture;
						const STexture & textureNormal = *material.m_hTexture2;

						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(nullptr, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferObject, m_cbufferGlobals };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &m_hMeshQuad->m_cbufferVertex, &m_hMeshQuad->m_cStride, &m_hMeshQuad->m_cOffset);

						{
							D3D11_MAPPED_SUBRESOURCE mappedSubresource;
							m_pD3ddevicecontext->Map(m_cbufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
							SGameObjectRenderConstants * pGorc = (SGameObjectRenderConstants *) (mappedSubresource.pData);
							pGorc->m_posCenter = hGo->m_pos;
							pGorc->m_vecScale = hGo->m_vecScale;
							pGorc->m_color = hGo->m_color;
							m_pD3ddevicecontext->Unmap(m_cbufferObject, 0);
						}

						ID3D11ShaderResourceView * aD3dsrview[] = { textureAlbedo.m_pD3dsrview, textureNormal.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { textureAlbedo.m_pD3dsamplerstate, textureNormal.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(m_hMeshQuad->m_cVerts, 0);
					}
					break;

				case TYPEK_Text:
					{
						const SMesh & mesh = *hGo->m_hMesh;
						const STexture & texture = *material.m_hTexture;

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(g_pBlendStateNoBlend, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferObject, m_cbufferGlobals };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SGameObjectRenderConstants * pGorc = (SGameObjectRenderConstants *) (mappedSubresource.pData);
						pGorc->m_posCenter = hGo->m_pos;
						pGorc->m_vecScale = hGo->m_vecScale;
						pGorc->m_color = hGo->m_color;
						m_pD3ddevicecontext->Unmap(m_cbufferObject, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_cVerts, 0);
					}
					break;
				default:
					{
						const SMesh & mesh = *hGo->m_hMesh;
						const STexture & texture = *material.m_hTexture;

						float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
						UINT sampleMask   = 0xffffffff;
						m_pD3ddevicecontext->OMSetBlendState(nullptr, blendFactor, sampleMask);

						m_pD3ddevicecontext->IASetInputLayout(shader.m_pD3dinputlayout);
						m_pD3ddevicecontext->VSSetShader(shader.m_pD3dvertexshader, nullptr, 0);
						m_pD3ddevicecontext->PSSetShader(shader.m_pD3dfragshader, nullptr, 0);

						m_pD3ddevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						ID3D11Buffer * aD3dbuffer[] = { m_cbufferObject, m_cbufferGlobals };
						m_pD3ddevicecontext->VSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->PSSetConstantBuffers(0, DIM(aD3dbuffer), aD3dbuffer);
						m_pD3ddevicecontext->IASetVertexBuffers(0, 1, &mesh.m_cbufferVertex, &mesh.m_cStride, &mesh.m_cOffset);

						D3D11_MAPPED_SUBRESOURCE mappedSubresource;
						m_pD3ddevicecontext->Map(m_cbufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
						SGameObjectRenderConstants * pGorc = (SGameObjectRenderConstants *) (mappedSubresource.pData);
						pGorc->m_posCenter = hGo->m_pos;
						pGorc->m_vecScale = hGo->m_vecScale;
						pGorc->m_color = hGo->m_color;
						m_pD3ddevicecontext->Unmap(m_cbufferObject, 0);

						ID3D11ShaderResourceView * aD3dsrview[] = { texture.m_pD3dsrview };
						m_pD3ddevicecontext->PSSetShaderResources(0, DIM(aD3dsrview), aD3dsrview);

						ID3D11SamplerState * aD3dsamplerstate[] = { texture.m_pD3dsamplerstate };
						m_pD3ddevicecontext->PSSetSamplers(0, DIM(aD3dsamplerstate), aD3dsamplerstate);

						m_pD3ddevicecontext->Draw(mesh.m_cVerts, 0);
					}
					break;
			}
		}

		// BB Do something nicer here?

		std::vector<SGameObjectHandle> aryhGoNext;
		for (SGameObjectHandle hGo : m_aryhGo)
		{
			if (hGo.PT())
				aryhGoNext.push_back(hGo);
		}
		m_aryhGo = aryhGoNext;

		std::vector<SScrabbleTileHandle> aryhTileNext;
		for (SScrabbleTileHandle hTile : m_aryhTile)
		{
			if (hTile.PT())
				aryhTileNext.push_back(hTile);
		}
		m_aryhTile = aryhTileNext;

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

#if TESTING_WORD_SCORES
void SGame::SetShowMovescore(int iMovescoreNew)
{
	if (iMovescoreNew == m_iMoveWordscore)
		return;

	m_iMoveWordscore = iMovescoreNew;

	for (SScrabbleTileHandle hTile : g_game.m_aryhTileWordPreview)
	{
		m_hGridBoard->m_ahTileGrid[hTile->m_iGrid] = -1;
		auto element = std::find(m_hGridBoard->m_aryhTileRack.begin(), m_hGridBoard->m_aryhTileRack.end(), hTile);
		if (element != m_hGridBoard->m_aryhTileRack.end())
			m_hGridBoard->m_aryhTileRack.erase(element);
		delete hTile.PT();
	}
	g_game.m_aryhTileWordPreview.clear();

	const SMove & move = g_game.m_aryMoveWordscore[m_iMoveWordscore];
	int x = move.m_x;
	int y = move.m_y;
	for (int i = move.m_iChMic; i < move.m_iChMac; i++)
	{
		char ch = move.m_aCh[i];
		SScrabbleTileHandle hTile = m_hGridBoard->HTileSpawn(ch);
		hTile->m_gSort = 100.0;
		m_hGridBoard->AddTileToGrid(hTile, x, y);
		if (move.m_fIsHorizontal)
		{
			x++;
		}
		else
		{
			y++;
		}
		m_aryhTileWordPreview.push_back(hTile);
	}
}
#endif

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
