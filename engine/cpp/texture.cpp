#include "texture.h"

#include "engine.h"
#include "external/stb_image.h"

STexture::STexture(const char * pChzFilename, bool fIsNormal, bool fGenerateMips, TYPEK typek) : super(typek)
{
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

	// Load Image
	int texNumChannels;
	int texForceNumChannels = 4;
	unsigned char * aBTexture = stbi_load(StrPrintf("%s\\%s", g_game.m_strAssetPath.c_str(), pChzFilename).c_str(), &m_dX, &m_dY,
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

STexture::STexture(TYPEK typek) : super(typek)
{
}
