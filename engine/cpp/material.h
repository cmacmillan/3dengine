#pragma once

#include "object.h"
#include "vector.h"

struct SNamedTexture
{
	STextureHandle	m_hTexture;
	std::string		m_strName;
};

struct SMaterial : SObject // material
{
	typedef SObject super;
	SMaterial(SShaderHandle hShader);
	SMaterialHandle HMaterial() { return (SMaterialHandle) m_nHandle; }

	std::vector<SNamedTexture> m_aryNamedtexture = {};
	SShaderHandle m_hShader = -1;
	float2 m_uvTopleft;
};
