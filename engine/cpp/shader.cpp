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

const std::string * PStrFind(const std::string & strKey, const std::vector<SPair> & aryPair)
{
	for (const SPair & pair : aryPair)
	{
		if (FMatchCaseInsensitive(pair.m_strKey, strKey))
			return &pair.m_strValue;
	}

	return nullptr;
}

bool FMatches(const char * pChBuilder, int cChBuilder, const char * pChzRef, int cChRef)
{
	if (cChBuilder != cChRef)
		return false;

	for (int iCh = 0; iCh < cChBuilder; iCh++)
	{
		if (pChBuilder[iCh] != pChzRef[iCh])
			return false;
	}

	return true;
}

SShader::SShader(const char * pChzFile) : super()
{
	m_typek = TYPEK_Shader;

	SFile file;
	VERIFY(FTryReadFile(pChzFile, &file));

	std::string strFile = file.StrGet();

	int iChBuilder = 0;
	char aChBuilder[1024];

	enum PARSE
	{
		PARSE_CommentStart = 0,
		PARSE_Key = 1,
		PARSE_Colon = 2,
		PARSE_Value = 3,
		PARSE_LineEnd = 4,
	};
	
	std::vector<SPair> aryPair;

	PARSE parse = PARSE_CommentStart;
	bool fSkipWhitespace = true;
	for (int iCh = 0;;)
	{
		if (iCh >= strFile.size() - 1)
		{
			ASSERT(false); // failed to find END_INFO
			return;
		}

		char ch = strFile[iCh];

		if (fSkipWhitespace)
		{
			if (FIsWhitespace(ch))
			{
				iCh++;
				continue;
			}

			fSkipWhitespace = false;
		}

		aChBuilder[iChBuilder++] = ch;

		const char * pChzTerminator = "END_INFO";
		bool fDone = false;
		switch (parse)
		{
			case PARSE_Key:
			case PARSE_Value:
				if (FMatches(aChBuilder, iChBuilder, pChzTerminator, strlen(pChzTerminator)))
				{
					fDone = true;
				}
				break;
		}
		if (fDone)
			break;

		switch (parse)
		{
			case PARSE_CommentStart:
				{
					const char * pChzToken = "//";
					if (FMatches(aChBuilder, iChBuilder, pChzToken, strlen(pChzToken)))
					{
						fSkipWhitespace = true;
						parse = PARSE_Key;
						iChBuilder = 0;
					}
					iCh++;
				}
				break;

			case PARSE_Key:
				{
					if (ch == ':' || FIsWhitespace(ch))
					{
						parse = PARSE_Colon;
						aryPair.push_back({ std::string(aChBuilder,iChBuilder-1), std::string() });
						fSkipWhitespace = true;
						iChBuilder = 0;
					}
					else
					{
						iCh++;
					}
				}
				break;

			case PARSE_Colon:
				{
					if (ch == ':')
					{
						parse = PARSE_Value;
						iChBuilder = 0;
						fSkipWhitespace = true;
						iCh++;
					}
					else
					{
						ASSERT(false);
					}
				}
				break;

			case PARSE_Value:
				{
					if (FIsWhitespace(ch) || ch == '\r' || ch == '\n')
					{
						aryPair[aryPair.size() - 1].m_strValue = std::string(aChBuilder,iChBuilder-1);
						parse = PARSE_LineEnd;
						iChBuilder = 0;
						fSkipWhitespace = true;
					}
					else
					{
						iCh++;
					}
				}
				break;

			case PARSE_LineEnd:
				{
					if (ch == '\r')
					{
						iCh++;
						ch = strFile[iCh];
					}

					if (ch == '\n')
					{
						parse = PARSE_CommentStart;
						fSkipWhitespace = true;
						iChBuilder = 0;
						iCh++;
					}
					else
					{
						ASSERT(false);
					}
				}
				break;
		}
	}

	static const std::string strShaderkKey = std::string("ShaderKind");
	static const std::string strShaderkValue3D = std::string("3D");
	static const std::string strShaderkValueUi = std::string("UI");

	const std::string * pStr = PStrFind("ShaderKind", aryPair);
	if (!pStr)
	{
		ASSERT(false); // No shader kind
	}
	else if (FMatchCaseInsensitive(*pStr, strShaderkValue3D))
	{
		m_shaderk = SHADERK_3D;
	}
	else if (FMatchCaseInsensitive(*pStr, strShaderkValueUi))
	{
		m_shaderk = SHADERK_Ui;
	}
	else
	{
		ASSERT(false); // Couldn't resolve type
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
