#include "shader.h"
#include <d3dcompiler.h>
#include "engine.h"
#include "file.h"
#include <vector>

struct SPair
{
	std::string m_strKey;
	std::string m_strValue;
};

struct SParsedLine
{
	const SPair * PPairParamFind(const std::string & str) const;

	SPair m_pairMain;
	std::vector<SPair> m_aryPair;
};

enum PARSE
{
	PARSE_CommentStart = 0,
	PARSE_Key = 1,
	PARSE_Delimiter = 2,
	PARSE_Value = 3,
	PARSE_LineEndOrParam = 4,
	PARSE_LineEnd = 5,
	PARSE_Done = 6,

	PARSE_Nil = -1,
};

struct SLineParser
{
	void						ParseLines(const std::string & str);
	void						SetState(PARSE parse, const std::string & str);
	void						FindLines(const std::string & strKey, std::vector<const SParsedLine *> * parypLine) const;

	std::vector<SParsedLine>	m_aryLine = {};
	PARSE						m_parse = PARSE_Nil;
	bool						m_fFoundMainPair = false;
	int							m_iChBuilder = -1;
	char						m_aChBuilder[256];
	int							m_iCh = -1;
};

const SPair * SParsedLine::PPairParamFind(const std::string & strKey) const
{
	for (const SPair & pair : m_aryPair)
	{
		if (FMatchCaseInsensitive(pair.m_strKey, strKey))
			return &pair;
	}
	return nullptr;
}

void SLineParser::FindLines(const std::string & strKey, std::vector<const SParsedLine *> * parypLine) const
{
	for (const SParsedLine & line: m_aryLine)
	{
		if (FMatchCaseInsensitive(line.m_pairMain.m_strKey, strKey))
			parypLine->push_back(&line);
	}

	return;
}

enum MATCHRES
{
	MATCHRES_PartialMatch, // All of builder matches the start of ref
	MATCHRES_Match, // All of builder matches all of ref
	MATCHRES_Mismatch, // All of builder matches all of ref
};

MATCHRES Matchres(const char * pChBuilder, int cChBuilder, const char * pChzRef, int cChRef)
{
	if (cChBuilder > cChRef)
		return MATCHRES_Mismatch;

	for (int iCh = 0; iCh < cChBuilder; iCh++)
	{
		if (pChBuilder[iCh] != pChzRef[iCh])
			return MATCHRES_Mismatch;
	}
	
	if (cChBuilder < cChRef)
	{
		return MATCHRES_PartialMatch;
	}

	return MATCHRES_Match;
}

void SLineParser::SetState(PARSE parse, const std::string & str)
{
	if (parse == m_parse)
		return;

	// Leave previous state

	switch (m_parse)
	{
		case PARSE_LineEnd:
		case PARSE_Delimiter:
		case PARSE_CommentStart:
			m_iCh++;
			break;

		case PARSE_Key:
			if (!m_fFoundMainPair)
			{
				m_aryLine.back().m_pairMain.m_strKey = std::string(m_aChBuilder, m_iChBuilder);
			}
			else
			{
				m_aryLine.back().m_aryPair.back().m_strKey = std::string(m_aChBuilder, m_iChBuilder);
			}
			break;

		case PARSE_Value:
			if (!m_fFoundMainPair)
			{
				m_aryLine.back().m_pairMain.m_strValue = std::string(m_aChBuilder, m_iChBuilder);
				m_fFoundMainPair = true;
			}
			else
			{
				m_aryLine.back().m_aryPair.back().m_strValue = std::string(m_aChBuilder, m_iChBuilder);
			}
			break;
	}

	PARSE parsePrev = m_parse;
	m_parse = parse;

	// Enter new State

	switch (m_parse)
	{
		case PARSE_Done:
			m_aryLine.pop_back();
			break;

		case PARSE_CommentStart:
			m_fFoundMainPair = false;
			break;

		case PARSE_Key:
			if (!m_fFoundMainPair)
			{
				m_aryLine.push_back({ });
			}
			else
			{
				m_aryLine.back().m_aryPair.push_back({});
			}
			break;

	}

	for (; FIsWhitespace(str[m_iCh]); m_iCh++);

	m_iChBuilder = 0;
}

void SLineParser::ParseLines(const std::string & str)
{
	ASSERT(m_parse == PARSE_Nil);

	m_iCh = 0;
	SetState(PARSE_CommentStart, str);
	
	for (;m_parse != PARSE_Done;)
	{
		if (m_iCh >= str.size() - 1)
		{
			ASSERT(false); // failed to find END_INFO
			return;
		}

		const char * pChzTerminator = "END_INFO";

		// Update state until it settles

		for (;;)
		{
			PARSE parsePrev = m_parse;

			char ch = str[m_iCh];

			if (ch == ';')
			{
				for (; str[m_iCh] != '\n'; m_iCh++);

				// Janky subtract since these will bump us forward when we leave them

				switch (m_parse)
				{
					case PARSE_LineEnd:
					case PARSE_Delimiter:
					case PARSE_CommentStart:
						m_iCh--;
						break;
				}

				SetState(PARSE_LineEnd, str);
				continue;
			}

			if (Matchres(m_aChBuilder, m_iChBuilder, pChzTerminator, strlen(pChzTerminator)) == MATCHRES_Match)
			{
				SetState(PARSE_Done, str);
			}

			char chParamDelimit = m_fFoundMainPair ? '=' : ':';
			switch (m_parse)
			{
				case PARSE_CommentStart:
					{
						const char * pChzToken = "//";
						MATCHRES matchres = Matchres(m_aChBuilder, m_iChBuilder, pChzToken, strlen(pChzToken));
						switch (matchres)
						{
							case MATCHRES_Match:
								SetState(PARSE_Key, str);
								break;

							case MATCHRES_Mismatch:
								ASSERT(false);
								break;
						}
					}
					break;

				case PARSE_Key:
					{
						if (ch == chParamDelimit || FIsWhitespace(ch))
							SetState(PARSE_Delimiter, str);
					}
					break;

				case PARSE_Delimiter:
					{
						if (ch == chParamDelimit)
							SetState(PARSE_Value, str);
					}
					break;

				case PARSE_Value:
					{
						if (FIsWhitespace(ch))
							SetState(PARSE_LineEndOrParam, str);
						if (ch == '\r' || ch == '\n')
							SetState(PARSE_LineEnd, str);
					}
					break;

				case PARSE_LineEndOrParam:
					{
						if (ch == '\r' || ch == '\n')
							SetState(PARSE_LineEnd, str);
						else
							SetState(PARSE_Key, str);
					}
					break;

				case PARSE_LineEnd:
					{
						if (ch == '\n')
						{
							SetState(PARSE_CommentStart, str);
						}
						else if (ch != '\r')
						{
							ASSERT(false);
						}
					}
					break;
			}

			if (parsePrev == m_parse)
				break;
		}

		m_aChBuilder[m_iChBuilder++] = str[m_iCh];
		m_iCh++;
	}


	SetState(PARSE_Nil, str);
}

SShader::SShader(const char * pChzFile) : super()
{
	m_typek = TYPEK_Shader;

	SFile file;
	VERIFY(FTryReadFile(pChzFile, &file));

	std::string strFile = file.StrGet();

	SLineParser parser;
	parser.ParseLines(strFile);

	static const std::string strShaderkKey = std::string("ShaderKind");
	static const std::string strShaderkValue3D = std::string("3D");
	static const std::string strShaderkValueUi = std::string("UI");

	std::vector<const SParsedLine *> arypLine;

	{
		parser.FindLines("ShaderKind", &arypLine);
		ASSERT(arypLine.size() == 1); // Not exactly 1 shader kind

		const SParsedLine & line = *arypLine.front();

		if (FMatchCaseInsensitive(line.m_pairMain.m_strValue, strShaderkValue3D))
		{
			m_shaderk = SHADERK_3D;
		}
		else if (FMatchCaseInsensitive(line.m_pairMain.m_strValue, strShaderkValueUi))
		{
			m_shaderk = SHADERK_Ui;
		}
		else
		{
			ASSERT(false); // Couldn't resolve type
		}
	}

	arypLine.clear();

	{
		parser.FindLines("Texture", &arypLine);

		std::vector<SNamedTextureSlot> aryNamedslot;
		for (const SParsedLine * pLine : arypLine)
		{
			if (const SPair * pPair = pLine->PPairParamFind("slot"))
			{
				aryNamedslot.push_back({ pLine->m_pairMain.m_strValue, NFromStr(pPair->m_strValue) });
			}
		}

		m_mpISlotStrName.resize(aryNamedslot.size());
		int iSlotMax = -1;
		for (int i = 0; i < aryNamedslot.size(); i++)
		{
			m_mpISlotStrName[i] = { {},-1 };
			iSlotMax = NMax(iSlotMax, aryNamedslot[i].m_iSlot);
		}
		ASSERT(iSlotMax + 1 == aryNamedslot.size());

		for (const SNamedTextureSlot & namedslot : aryNamedslot)
		{
			m_mpISlotStrName[namedslot.m_iSlot] = namedslot;
		}

		for (const SNamedTextureSlot & namedslot : m_mpISlotStrName)
		{
			ASSERT(namedslot.m_iSlot != -1);
		}
	}

	// Create Vertex Shader
	ID3DBlob * vsBlob = nullptr;
	{
		// BB Omitting 3rd argument to D3DCompile will prevent shader #includes from working

		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompile(file.m_pB, file.m_cBytesFile, nullptr, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			if (shaderCompileErrorsBlob)
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
		// BB Omitting 3rd argument to D3DCompile will prevent shader #includes from working

		ID3DBlob * psBlob;
		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompile(file.m_pB, file.m_cBytesFile, nullptr, nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char * errorString = NULL;
			if (shaderCompileErrorsBlob)
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

	if (m_shaderk == SHADERK_3D)
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
		ASSERT(m_shaderk == SHADERK_Ui);

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
