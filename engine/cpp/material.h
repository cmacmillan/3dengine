#pragma once

#include "object.h"

struct SMaterial : SObject // material
{
	typedef SObject super;
	SMaterial(SShaderHandle hShader);
	SMaterialHandle HMaterial() { return (SMaterialHandle) m_nHandle; }

	STextureHandle m_hTexture = -1;
	STextureHandle m_hTexture2 = -1;
	SShaderHandle m_hShader = -1;
	float2 m_uvTopleft;
};
