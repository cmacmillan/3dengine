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
	SMaterial(SShaderHandle hShader, const char * pChzName = "", TYPEK typek = TYPEK_Material);
	SMaterialHandle HMaterial() { return (SMaterialHandle) m_h; }

	std::vector<SNamedTexture> m_aryNamedtexture = {};
	std::string m_strName = "";
	SShaderHandle m_hShader = -1;
	float2 m_uvTopleft;
};
