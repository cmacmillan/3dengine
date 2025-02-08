#include "material.h"

SMaterial::SMaterial(SShaderHandle hShader, const char * pChzName, TYPEK typek) : m_strName(pChzName), m_hShader(hShader), super(typek)
{
}