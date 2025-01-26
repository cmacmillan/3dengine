#include "shader.h"
#include <d3dcompiler.h>
#include "engine.h"
#include "file.h"
#include <vector>
#include "linearmap.h"

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

	static SLinearMap<std::string, BOOL, 2> mpStrBool = { {
														{std::string("On"), TRUE},
														{std::string("Off"), FALSE}} };

	SLineParser parser;
	parser.ParseLines(strFile);

	std::vector<const SParsedLine *> arypLine;


	// Set up defaults:

	m_d3ddepthstencildesc.DepthFunc = D3D11_COMPARISON_GREATER; // default to greater since our clip space znear=1.0, zfar=0.0
	m_d3ddepthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_d3ddepthstencildesc.DepthEnable = FALSE;

	m_d3drasterizerdesc.CullMode = D3D11_CULL_BACK;
	m_d3drasterizerdesc.FillMode = D3D11_FILL_SOLID;
	m_d3drasterizerdesc.FrontCounterClockwise = TRUE;

	m_d3drtblenddesc.BlendEnable = FALSE;
	m_d3drtblenddesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_d3drtblenddesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_d3drtblenddesc.BlendOp = D3D11_BLEND_OP_ADD;
	m_d3drtblenddesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_d3drtblenddesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_d3drtblenddesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	m_d3drtblenddesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

	m_d3drtblenddesc.LogicOpEnable = FALSE;
	m_d3drtblenddesc.LogicOp = D3D11_LOGIC_OP_CLEAR;

	const char * pChzShaderKind		= "ShaderKind";
	const char * pChzDepthEnable	= "DepthEnable";
	const char * pChzDepthWrite		= "DepthWrite";
	const char * pChzDepthFunc		= "DepthFunc";
	const char * pChzFillMode		= "FillMode";
	const char * pChzCullMode		= "CullMode";
	const char * pChzBlendEnable	= "BlendEnable";
	const char * pChzSrcBlend		= "SrcBlend";
	const char * pChzDestBlend		= "DestBlend";
	const char * pChzBlendOp		= "BlendOp";
	const char * pChzRtWriteMask	= "RtWriteMask";
	const char * pChzBlendOpAlpha	= "BlendOpAlpha";
	const char * pChzSrcBlendAlpha	= "SrcBlendAlpha";
	const char * pChzDestBlendAlpha = "DestBlendAlpha";
	const char * pChzTexture		= "Texture";

	const char * apChzKeys[] = { 
								pChzShaderKind, 
								pChzDepthEnable, 
								pChzDepthWrite, 
								pChzDepthFunc,
								pChzFillMode,
								pChzCullMode,
								pChzBlendEnable,
								pChzSrcBlend,
								pChzDestBlend,
								pChzBlendOp,
								pChzRtWriteMask,
								pChzBlendOpAlpha,
								pChzSrcBlendAlpha,
								pChzDestBlendAlpha,
								pChzTexture};

	for (const SParsedLine & line : parser.m_aryLine)
	{
		bool fFound = false;
		for (int i = 0; i < DIM(apChzKeys); i++)
		{
			if (FMatchCaseInsensitive(line.m_pairMain.m_strKey, apChzKeys[i]))
			{
				fFound = true;
				break;
			}
		}

		VERIFY(fFound); // Unknown keyword
	}

	// Shaderkind

	{
		parser.FindLines(pChzShaderKind, &arypLine);
		static SLinearMap<std::string, SHADERK, 3> mpStrShaderk = {{
																{std::string("3D"), SHADERK_3D}, 
																{std::string("UI"), SHADERK_Ui}, 
																{std::string("Skybox"), SHADERK_Skybox}}};
			
		ASSERT(arypLine.size() == 1); // Not exactly 1 shader kind

		VERIFY(mpStrShaderk.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_shaderk));
		arypLine.clear();
	}

	// Depth Enable

	{
		parser.FindLines(pChzDepthEnable, &arypLine);
		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBool.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3ddepthstencildesc.DepthEnable));
			arypLine.clear();
		}
	}

	// Depth Write

	{	
		parser.FindLines(pChzDepthWrite, &arypLine);
		static SLinearMap<std::string, D3D11_DEPTH_WRITE_MASK, 2> mpStrD3ddepthwritemask = { {
															{std::string("On"), D3D11_DEPTH_WRITE_MASK_ALL},
															{std::string("Off"), D3D11_DEPTH_WRITE_MASK_ZERO}} };
		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrD3ddepthwritemask.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3ddepthstencildesc.DepthWriteMask));
			arypLine.clear();
		}
	}

	// Depth Function

	{
		parser.FindLines(pChzDepthFunc, &arypLine);
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_comparison_func

		static SLinearMap<std::string, D3D11_COMPARISON_FUNC, 8> mpStrComparisonfunc = {{
																{std::string("Never"), D3D11_COMPARISON_NEVER}, 
																{std::string("Less"), D3D11_COMPARISON_LESS}, 
																{std::string("Equal"), D3D11_COMPARISON_EQUAL},
																{std::string("LessEqual"), D3D11_COMPARISON_LESS_EQUAL},
																{std::string("Greater"), D3D11_COMPARISON_GREATER},
																{std::string("NotEqual"), D3D11_COMPARISON_NOT_EQUAL},
																{std::string("GreaterEqual"), D3D11_COMPARISON_GREATER_EQUAL},
																{std::string("Always"), D3D11_COMPARISON_ALWAYS}}};
		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrComparisonfunc.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3ddepthstencildesc.DepthFunc));
			arypLine.clear();
		}
	}

	// FillMode

	{
		parser.FindLines(pChzFillMode, &arypLine);
		static SLinearMap<std::string, D3D11_FILL_MODE, 2> mpStrFillmode = {{
																{std::string("Wireframe"), D3D11_FILL_WIREFRAME}, 
																{std::string("Solid"), D3D11_FILL_SOLID}}};
		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrFillmode.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drasterizerdesc.FillMode));
			arypLine.clear();
		}
	}

	// CullMode

	{
		parser.FindLines(pChzCullMode, &arypLine);
		static SLinearMap<std::string, D3D11_CULL_MODE, 3> mpStrCullmode = {{
																{std::string("None"), D3D11_CULL_NONE}, 
																{std::string("Front"), D3D11_CULL_FRONT},
																{std::string("Back"), D3D11_CULL_BACK}}};
		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrCullmode.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drasterizerdesc.CullMode));
			arypLine.clear();
		}
	}

	static SLinearMap<std::string, D3D11_BLEND, 17> mpStrBlend = {{
															{std::string("Zero"),			D3D11_BLEND_ZERO}, 
															{std::string("One"),			D3D11_BLEND_ONE},
															{std::string("SrcColor"),		D3D11_BLEND_SRC_COLOR},
															{std::string("InvSrcColor"),	D3D11_BLEND_INV_SRC_COLOR},
															{std::string("SrcAlpha"),		D3D11_BLEND_SRC_ALPHA},
															{std::string("InvSrcAlpha"),	D3D11_BLEND_INV_SRC_ALPHA},
															{std::string("DestAlpha"),		D3D11_BLEND_DEST_ALPHA},
															{std::string("InvDestAlpha"),	D3D11_BLEND_INV_DEST_ALPHA},
															{std::string("DestColor"),		D3D11_BLEND_DEST_COLOR},
															{std::string("InvDestColor"),	D3D11_BLEND_INV_DEST_COLOR},
															{std::string("SrcAlphaSat"),	D3D11_BLEND_SRC_ALPHA_SAT},
															{std::string("BlendFactor"),	D3D11_BLEND_BLEND_FACTOR},
															{std::string("InvBlendFactor"), D3D11_BLEND_INV_BLEND_FACTOR},
															{std::string("Src1Color"),		D3D11_BLEND_SRC1_COLOR},
															{std::string("InvSrc1Color"),	D3D11_BLEND_INV_SRC1_COLOR},
															{std::string("Src1Alpha"),		D3D11_BLEND_SRC1_ALPHA},
															{std::string("InvSrc1Alpha"),	D3D11_BLEND_INV_SRC1_ALPHA}}};

	static SLinearMap<std::string, D3D11_BLEND_OP, 5> mpStrBlendop = {{
															{std::string("Add"), D3D11_BLEND_OP_ADD}, 
															{std::string("Subtract"), D3D11_BLEND_OP_SUBTRACT},
															{std::string("RevSubtract"), D3D11_BLEND_OP_REV_SUBTRACT},
															{std::string("Min"), D3D11_BLEND_OP_MIN},
															{std::string("Max"), D3D11_BLEND_OP_MAX}}};

	// BlendEnable

	{
		parser.FindLines(pChzBlendEnable, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBool.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.BlendEnable));
			arypLine.clear();
		}
	}

	// SrcBlend 	

	{
		parser.FindLines(pChzSrcBlend, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlend.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.SrcBlend));
			arypLine.clear();
		}
	}

	// DestBlend

	{
		parser.FindLines(pChzDestBlend, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlend.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.DestBlend));
			arypLine.clear();
		}
	}

	// BlendOp

	{
		parser.FindLines(pChzBlendOp, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlendop.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.BlendOp));
			arypLine.clear();
		}
	}

	// RenderTargetWriteMask

	{
		parser.FindLines(pChzRtWriteMask, &arypLine);
		static SLinearMap<std::string, UINT8, 16> mpStrColorwriteenable = { {
																{std::string("None"),	0},
																{std::string("R"),		D3D11_COLOR_WRITE_ENABLE_RED },
																{std::string("G"),		D3D11_COLOR_WRITE_ENABLE_GREEN },
																{std::string("B"),		D3D11_COLOR_WRITE_ENABLE_BLUE },
																{std::string("A"),		D3D11_COLOR_WRITE_ENABLE_ALPHA },
																{std::string("RG"),		D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN },
																{std::string("RB"),		D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE },
																{std::string("RA"),		D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_ALPHA },
																{std::string("GB"),		D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE},
																{std::string("GA"),		D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_ALPHA},
																{std::string("BA"),		D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA},
																{std::string("RGB"),	D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE},
																{std::string("RBA"),    D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA},
																{std::string("RGA"),	D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_ALPHA},
																{std::string("GBA"),	D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA},
																{std::string("RGBA"),	D3D11_COLOR_WRITE_ENABLE_ALL}} };

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrColorwriteenable.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.RenderTargetWriteMask));
			arypLine.clear();
		}
	}

	// BlendOpAlpha

	{
		parser.FindLines(pChzBlendOpAlpha, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlendop.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.BlendOpAlpha));
			arypLine.clear();
		}
	}

	// SrcBlendAlpha

	{
		parser.FindLines(pChzSrcBlendAlpha, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlend.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.SrcBlendAlpha));
			arypLine.clear();
		}
	}

	// DestBlendAlpha

	{
		parser.FindLines(pChzDestBlendAlpha, &arypLine);

		if (arypLine.size() > 0)
		{
			ASSERT(arypLine.size() == 1); // Not exactly 1

			VERIFY(mpStrBlend.FTryGetValueFromKey(FMatchCaseInsensitive, arypLine.front()->m_pairMain.m_strValue, &m_d3drtblenddesc.DestBlendAlpha));
			arypLine.clear();
		}
	}

	// Texture

	{
		parser.FindLines(pChzTexture, &arypLine);

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

		arypLine.clear();
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

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT hResult = g_game.m_pD3ddevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pD3dinputlayout);
	assert(SUCCEEDED(hResult));
	vsBlob->Release();

	// Create rasterizer state

	g_game.m_pD3ddevice->CreateRasterizerState(&m_d3drasterizerdesc, &m_pD3drasterizerstate);

	// Create depth stencil state

	g_game.m_pD3ddevice->CreateDepthStencilState(&m_d3ddepthstencildesc, &m_pD3ddepthstencilstate);

	// Create blend state

	{
		D3D11_BLEND_DESC1 BlendState;
		ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
		D3D11_RENDER_TARGET_BLEND_DESC1 * pD3drtbd = &BlendState.RenderTarget[0];

		*pD3drtbd = m_d3drtblenddesc;

		pD3drtbd->LogicOpEnable = FALSE;
		pD3drtbd->LogicOp = D3D11_LOGIC_OP_CLEAR;

		g_game.m_pD3ddevice->CreateBlendState1(&BlendState, &m_pD3dblendstatenoblend);
	}
}
