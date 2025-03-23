#include "shader.h"
#include <d3dcompiler.h>
#include "engine.h"
#include "file.h"
#include <vector>

#pragma comment(lib, "dxguid.lib") // For shader reflection

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

// BB $replacement doesn't work with textures

bool FTryPerformReplacement(SKv<std::string, std::string> * aKvReplacement, int cKvReplacement, std::string * pStrReplace, std::string * pStrError)
{
	if (!aKvReplacement)
		return true;

	// BB this replacement using a kv map filled with strings is gross & very bad

	if (pStrReplace->size() == 0)
		return true;

	if ((*pStrReplace)[0] != '$')
		return true;

	*pStrReplace = pStrReplace->substr(1); // Remove the '$'

	// If str value starts with dollar sign, perform replacement

	VERIFY(FTryGetValueFromKey(aKvReplacement, cKvReplacement, FMatchCaseInsensitive, *pStrReplace, pStrReplace));
}

template<typename K, typename V>
bool FTryParseOneOrZeroParams(const SLineParser & parser, const char * pChzParameterName, SKv<K, V> * aKv, int cKv, SKv<std::string, std::string> * aKvReplacement, int cKvReplacement, V * pValue, std::string * pStrError)
{
	std::vector<const SParsedLine *> arypLine; // TODO turn into stack or frame array

	parser.FindLines(pChzParameterName, &arypLine);

	if (arypLine.size() > 0)
	{
		if (arypLine.size() != 1)
		{
			*pStrError = StrPrintf("More than one '%s' tag", pChzParameterName);
			return false;
		}

		std::string strValue = arypLine.front()->m_pairMain.m_strValue;
		if (!FTryPerformReplacement(aKvReplacement, cKvReplacement, &strValue, pStrError))
			return false;

		if (!FTryGetValueFromKey(aKv, cKv, FMatchCaseInsensitive, strValue, pValue))
		{
			*pStrError = StrPrintf("Unrecognized '%s' tag '%s'", pChzParameterName, strValue.c_str());
			return false;
		}
	}

	return true;
}

bool SShader::FTryLoadFromFile(SFile * pFile, SKv<std::string, std::string> * aKvReplacement, int cKvReplacement, SShaderData * pData, std::string * pStrError)
{
	std::string strFile = pFile->StrGet();

	static SKv<std::string, BOOL> mpStrBool[] = {	{std::string("On"), TRUE},
													{std::string("Off"), FALSE}};

	SLineParser parser;
	parser.ParseLines(strFile);

	// Set up defaults:

	pData->m_d3ddepthstencildesc.DepthFunc = D3D11_COMPARISON_GREATER; // default to greater since our clip space znear=1.0, zfar=0.0
	pData->m_d3ddepthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	pData->m_d3ddepthstencildesc.DepthEnable = FALSE;

	pData->m_d3drasterizerdesc.CullMode = D3D11_CULL_BACK;
	pData->m_d3drasterizerdesc.FillMode = D3D11_FILL_SOLID;
	pData->m_d3drasterizerdesc.FrontCounterClockwise = TRUE;
	pData->m_d3drasterizerdesc.DepthClipEnable = TRUE;

	pData->m_d3drtblenddesc.BlendEnable = FALSE;
	pData->m_d3drtblenddesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	pData->m_d3drtblenddesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	pData->m_d3drtblenddesc.BlendOp = D3D11_BLEND_OP_ADD;
	pData->m_d3drtblenddesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	pData->m_d3drtblenddesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	pData->m_d3drtblenddesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	pData->m_d3drtblenddesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;

	pData->m_d3drtblenddesc.LogicOpEnable = FALSE;
	pData->m_d3drtblenddesc.LogicOp = D3D11_LOGIC_OP_CLEAR;

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
	const char * pChzShadowcast		= "Shadowcast";

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
								pChzTexture,
								pChzShadowcast };

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

		if (!fFound)
		{
			*pStrError = StrPrintf("Unknown keyword '%s'", line.m_pairMain.m_strKey.c_str());
			return false;
		}
	}

	// Shaderkind

	{
		static SKv<std::string, SHADERK> mpStrShaderk[] = {	{std::string("3D"), SHADERK_3D},
															{std::string("UI"), SHADERK_Ui}, 
															{std::string("Skybox"), SHADERK_Skybox}};
			
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzShaderKind, 
				mpStrShaderk, 
				DIM(mpStrShaderk), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_shaderk, 
				pStrError))
			return false;
	}

	// Shadowcast

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzShadowcast, 
				mpStrBool, 
				DIM(mpStrBool), 
				aKvReplacement,
				cKvReplacement,
				(BOOL *)&pData->m_fShadowcast,
				pStrError))
			return false;
	}

	// Depth Enable

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzDepthEnable, 
				mpStrBool, 
				DIM(mpStrBool), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3ddepthstencildesc.DepthEnable,
				pStrError))
			return false;
	}

	// Depth Write

	{	
		static SKv<std::string, D3D11_DEPTH_WRITE_MASK> mpStrD3ddepthwritemask[] = {{std::string("On"), D3D11_DEPTH_WRITE_MASK_ALL},
																					{std::string("Off"), D3D11_DEPTH_WRITE_MASK_ZERO} };

		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzDepthWrite, 
				mpStrD3ddepthwritemask, 
				DIM(mpStrD3ddepthwritemask), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3ddepthstencildesc.DepthWriteMask,
				pStrError))
			return false;
	}

	// Depth Function

	{
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_comparison_func

		static SKv<std::string, D3D11_COMPARISON_FUNC> mpStrComparisonfunc[] = {{std::string("Never"), D3D11_COMPARISON_NEVER}, 
																				{std::string("Less"), D3D11_COMPARISON_LESS}, 
																				{std::string("Equal"), D3D11_COMPARISON_EQUAL},
																				{std::string("LessEqual"), D3D11_COMPARISON_LESS_EQUAL},
																				{std::string("Greater"), D3D11_COMPARISON_GREATER},
																				{std::string("NotEqual"), D3D11_COMPARISON_NOT_EQUAL},
																				{std::string("GreaterEqual"), D3D11_COMPARISON_GREATER_EQUAL},
																				{std::string("Always"), D3D11_COMPARISON_ALWAYS}};

		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzDepthFunc, 
				mpStrComparisonfunc, 
				DIM(mpStrComparisonfunc), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3ddepthstencildesc.DepthFunc,
				pStrError))
			return false;
	}

	// FillMode

	{
		static SKv<std::string, D3D11_FILL_MODE> mpStrFillmode[] = {{std::string("Wireframe"), D3D11_FILL_WIREFRAME},
																	{std::string("Solid"), D3D11_FILL_SOLID}};

		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzFillMode, 
				mpStrFillmode, 
				DIM(mpStrFillmode), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drasterizerdesc.FillMode,
				pStrError))
			return false;
	}

	// CullMode

	{
		static SKv<std::string, D3D11_CULL_MODE> mpStrCullmode[] = {{std::string("None"), D3D11_CULL_NONE}, 
																	{std::string("Front"), D3D11_CULL_FRONT},
																	{std::string("Back"), D3D11_CULL_BACK}};
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzCullMode, 
				mpStrCullmode, 
				DIM(mpStrCullmode), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drasterizerdesc.CullMode,
				pStrError))
			return false;
	}

	static SKv<std::string, D3D11_BLEND> mpStrBlend[] = {	{std::string("Zero"),			D3D11_BLEND_ZERO},
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
															{std::string("InvSrc1Alpha"),	D3D11_BLEND_INV_SRC1_ALPHA}};

	static SKv<std::string, D3D11_BLEND_OP> mpStrBlendop[] = {	{std::string("Add"), D3D11_BLEND_OP_ADD},
																{std::string("Subtract"), D3D11_BLEND_OP_SUBTRACT},
																{std::string("RevSubtract"), D3D11_BLEND_OP_REV_SUBTRACT},
																{std::string("Min"), D3D11_BLEND_OP_MIN},
																{std::string("Max"), D3D11_BLEND_OP_MAX}};

	// BlendEnable

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzBlendEnable, 
				mpStrBool, 
				DIM(mpStrBool), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.BlendEnable,
				pStrError))
			return false;
	}

	// SrcBlend 	

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzSrcBlend, 
				mpStrBlend, 
				DIM(mpStrBlend), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.SrcBlend,
				pStrError))
			return false;
	}

	// DestBlend

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzDestBlend, 
				mpStrBlend, 
				DIM(mpStrBlend), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.DestBlend,
				pStrError))
			return false;
	}

	// BlendOp

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzBlendOp, 
				mpStrBlendop, 
				DIM(mpStrBlendop), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.BlendOp,
				pStrError))
			return false;
	}

	// RenderTargetWriteMask

	{
		static SKv<std::string, UINT8> mpStrColorwriteenable[] = {	{std::string("None"),	0},
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
																	{std::string("RGBA"),	D3D11_COLOR_WRITE_ENABLE_ALL}};
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzRtWriteMask, 
				mpStrColorwriteenable, 
				DIM(mpStrColorwriteenable), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.RenderTargetWriteMask,
				pStrError))
			return false;
	}

	// BlendOpAlpha

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzBlendOpAlpha, 
				mpStrBlendop, 
				DIM(mpStrBlendop), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.BlendOpAlpha,
				pStrError))
			return false;
	}

	// SrcBlendAlpha

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzSrcBlendAlpha, 
				mpStrBlend, 
				DIM(mpStrBlend), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.SrcBlendAlpha,
				pStrError))
			return false;
	}

	// DestBlendAlpha

	{
		if (!FTryParseOneOrZeroParams(
				parser, 
				pChzDestBlendAlpha, 
				mpStrBlend, 
				DIM(mpStrBlend), 
				aKvReplacement,
				cKvReplacement,
				&pData->m_d3drtblenddesc.DestBlendAlpha,
				pStrError))
			return false;
	}

	// Texture

	{
		std::vector<const SParsedLine *> arypLine; // TODO turn into stack or frame array

		parser.FindLines(pChzTexture, &arypLine);

		std::vector<SNamedTextureSlot> aryNamedslot;
		for (const SParsedLine * pLine : arypLine)
		{
			if (const SPair * pPair = pLine->PPairParamFind("slot"))
			{
				aryNamedslot.push_back({ pLine->m_pairMain.m_strValue, NFromStr(pPair->m_strValue) });
			}
		}

		pData->m_mpISlotStrName.resize(aryNamedslot.size());
		int iSlotMax = -1;
		for (int i = 0; i < aryNamedslot.size(); i++)
		{
			pData->m_mpISlotStrName[i] = { {},-1 };
			iSlotMax = NMax(iSlotMax, aryNamedslot[i].m_iSlot);
		}

		if (iSlotMax + 1 != aryNamedslot.size())
		{
			*pStrError = "Texture slot count mismatch!";
			return false;
		}

		for (const SNamedTextureSlot & namedslot : aryNamedslot)
		{
			pData->m_mpISlotStrName[namedslot.m_iSlot] = namedslot;
		}

		for (const SNamedTextureSlot & namedslot : pData->m_mpISlotStrName)
		{
			if (namedslot.m_iSlot == -1)
			{
				*pStrError = "Texture slot error!";
				return false;
			}
		}

		arypLine.clear();
	}

	// Create Vertex Shader
	ID3DBlob * vsBlob = nullptr;
	{
		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResultCompileVs = D3DCompile(
										pFile->m_pB, 
										pFile->m_cBytesFile, 
										pFile->m_strPath.c_str(),
										nullptr, 
										D3D_COMPILE_STANDARD_FILE_INCLUDE, 
										"vs_main", 
										"vs_5_0", 
										0, 
										0, 
										&vsBlob, 
										&shaderCompileErrorsBlob);
		if (FAILED(hResultCompileVs))
		{
			if (shaderCompileErrorsBlob)
			{
				*pStrError = (const char *) shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			return false;
		}
	}

	// Create Pixel Shader

	ID3DBlob * psBlob;
	{
		ID3DBlob * shaderCompileErrorsBlob;
		HRESULT hResultCompilePs = D3DCompile(
											pFile->m_pB, 
											pFile->m_cBytesFile, 
											pFile->m_strPath.c_str(),
											nullptr, 
											D3D_COMPILE_STANDARD_FILE_INCLUDE, 
											"ps_main", 
											"ps_5_0", 
											0, 
											0, 
											&psBlob, 
											&shaderCompileErrorsBlob);
		if (FAILED(hResultCompilePs))
		{
			if (shaderCompileErrorsBlob)
			{
				*pStrError = (const char *) shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			return false;
		}
	}

	// BEYOND THIS POINT, THE FUNCTION CANNOT FAIL OR RETURN FALSE. ERRORS SHOULD BE CRASHES OR ASSERTS

	HRESULT hResultCreateVs = g_game.m_pD3ddevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pData->m_pD3dvertexshader);
	assert(SUCCEEDED(hResultCreateVs));

	HRESULT hResultCreatePs = g_game.m_pD3ddevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pData->m_pD3dfragshader);
	assert(SUCCEEDED(hResultCreatePs));
	psBlob->Release();
	
#if 0 // BB force this to match with the input data
	ID3D11ShaderReflection* pReflector = NULL;
	D3DReflect(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &pReflector);

	D3D11_SHADER_DESC shaderdesc;
	pReflector->GetDesc(&shaderdesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> aryInputele;
	for (int iInput = 0; iInput < shaderdesc.InputParameters; iInput++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC sigdesc;
		pReflector->GetInputParameterDesc(iInput, &sigdesc);
		DoNothing();
	}
#endif

	// Create Input Layout

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT hResultInputLayout = g_game.m_pD3ddevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &pData->m_pD3dinputlayout);
	assert(SUCCEEDED(hResultInputLayout));

	vsBlob->Release();

	// Create rasterizer state

	g_game.m_pD3ddevice->CreateRasterizerState(&pData->m_d3drasterizerdesc, &pData->m_pD3drasterizerstate);

	// Create depth stencil state

	g_game.m_pD3ddevice->CreateDepthStencilState(&pData->m_d3ddepthstencildesc, &pData->m_pD3ddepthstencilstate);

	// Create blend state

	{
		D3D11_BLEND_DESC1 BlendState;
		ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
		D3D11_RENDER_TARGET_BLEND_DESC1 * pD3drtbd = &BlendState.RenderTarget[0];

		*pD3drtbd = pData->m_d3drtblenddesc;

		pD3drtbd->LogicOpEnable = FALSE;
		pD3drtbd->LogicOp = D3D11_LOGIC_OP_CLEAR;

		g_game.m_pD3ddevice->CreateBlendState1(&BlendState, &pData->m_pD3dblendstatenoblend);
	}

	return true;
}

SShader::SShader(const char * pChzFile, SKv<std::string, std::string> * aKvReplacement, int cKvReplacement, TYPEK typek) : super(typek)
{
	m_strFile = pChzFile;

	SFile file;
	VERIFY(FTryReadFile(pChzFile, &file));

	m_filetimeLastEdit = file.FiletimeLastWrite();

	SShaderData data;
	std::string strError;
	if (!FTryLoadFromFile(&file, aKvReplacement, cKvReplacement, &data, &strError))
	{
		m_data.m_shaderk = SHADERK_Error;
		g_game.PrintConsole(StrPrintf("Shader compile error (%s) :\n%s\n", m_strFile.c_str(), strError.c_str()), 20.0f);
		return;
	}

	m_aKvReplacement = aKvReplacement;
	m_cKvReplacement = cKvReplacement;
	m_data = data;
}

SShader::~SShader()
{
	ReleaseResources();
}

void SShader::UpdateHotload()
{
	SFile file;

	// If we fail to read, it's probably being written to, so ignore for now

	if (!FTryReadFile(m_strFile.c_str(), &file))
		return;

	FILETIME filetimeLastEdit = file.FiletimeLastWrite();

	if (CompareFileTime(&filetimeLastEdit, &m_filetimeLastEdit) <= 0)
		return;

	m_filetimeLastEdit = file.FiletimeLastWrite();

	g_game.PrintConsole("Recompiling shader...\n", 5.0f);

	SShaderData data;
	std::string strError;
	if (!FTryLoadFromFile(&file, m_aKvReplacement, m_cKvReplacement, &data, &strError))
	{
		m_data.m_shaderk = SHADERK_Error;
		g_game.PrintConsole(StrPrintf("Shader compile error (%s) :\n%s\n", m_strFile.c_str(), strError.c_str()), 15.0f);
		return;
	}

	ReleaseResources();
	
	m_data = data;
}

void SShader::ReleaseResources()
{
	m_data.m_pD3dvertexshader->Release();
	m_data.m_pD3dvertexshader = nullptr;

	m_data.m_pD3dfragshader->Release();
	m_data.m_pD3dfragshader = nullptr;

	m_data.m_pD3dinputlayout->Release();
	m_data.m_pD3dinputlayout = nullptr;

	m_data.m_pD3dblendstatenoblend->Release();
	m_data.m_pD3dblendstatenoblend = nullptr;

	m_data.m_pD3drasterizerstate->Release();
	m_data.m_pD3drasterizerstate = nullptr;

	m_data.m_pD3ddepthstencilstate->Release();
	m_data.m_pD3ddepthstencilstate = nullptr;
}
