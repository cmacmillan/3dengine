#include "drawnode3d.h"

SDrawNode3D::SDrawNode3D(SNode * pNodeParent, const std::string & str, TYPEK typek) :
	super(pNodeParent, str, typek)
{
}

void SDrawNodeRenderConstants::FillOut(const Mat & matObjectToWorld,  const Mat & matWorldToObject, const Mat & matWorldToClip, const Mat & matWorldToCamera, SRgba rgba)
{
	m_matMVP = matObjectToWorld * matWorldToClip;
	m_matVP = matWorldToClip;
	m_matV = matWorldToCamera;
	m_matObjectToWorld = matObjectToWorld;
	m_matWorldToObject = matWorldToObject;
	Mat matObjectToWorldNoTranslate = matObjectToWorld;
	matObjectToWorldNoTranslate.m_aVec[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
	m_matObjectToWorldInverseTranspose = matObjectToWorldNoTranslate.MatInverse().MatTranspose();

	// NOTE we don't have to do this with textures because they were written out in srgb

	m_rgba = RgbaSrgbFromLinear(rgba); 
}
