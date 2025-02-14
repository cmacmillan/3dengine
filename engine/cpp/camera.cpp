#include "camera3d.h"
#include "engine.h"

SCamera3D::SCamera3D(SNodeHandle hNodeParent,  const std::string & strName, float radFovHorizontal, float xNearClip, float xFarClip, TYPEK typek) :
	super(hNodeParent, strName, typek),
	m_radFovHorizontal(radFovHorizontal),
	m_xNearClip(xNearClip),
	m_xFarClip(xFarClip)
{
}

void SCamera3D::SetOrthographic(float gScaleOrthographic)
{
	m_fOrthographic = true;
	m_gScaleOrthographic = gScaleOrthographic;
}

Mat SCamera3D::MatCameraToClip()
{
	float2 vecWinSize = g_game.VecWinSize();
	if (m_fOrthographic)
		return MatOrthographic(m_gScaleOrthographic, vecWinSize.m_x / vecWinSize.m_y, m_xNearClip, m_xFarClip);
	else
		return MatPerspective(m_radFovHorizontal, vecWinSize.m_x / vecWinSize.m_y, m_xNearClip, m_xFarClip);
}

Mat SCamera3D::MatWorldToClip()
{
	return MatObjectToWorld().MatInverse() * MatCameraToClip();
}

Mat SCamera3D::MatClipToCamera()
{
	return MatCameraToClip().MatInverse();
}

Mat SCamera3D::MatClipToWorld()
{
	return MatCameraToClip() * MatObjectToWorld();
}
