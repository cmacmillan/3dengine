#include "material.h"

SMaterial::SMaterial(SShaderHandle hShader) : super()
{
	m_typek = TYPEK_Material;

	m_hShader = hShader;
}