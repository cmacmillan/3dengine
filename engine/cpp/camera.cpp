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

Point SCamera3D::PosWorldFromPosNdc(Point posNdc)
{
	// z near = 1, z far = 0, x & y range -1 to 1

	// Clamp to bounds

	posNdc = Point(VecComponentwiseMin(VecComponentwiseMax(Vector(posNdc), Vector(-1.0f, -1.0f, 0.0f)), Vector(1.0f, 1.0f, 1.0f)));

	Mat matClipToCamera = MatClipToCamera();

	float xCam = GMapRange(1.0f, 0.0f, m_xNearClip, m_xFarClip, posNdc.m_vec.m_z);

	// Multiplying by xCam since that's what ends up in the w component

	float4 posResult = (posNdc.m_vec * xCam) * matClipToCamera;

	// NOTE W isn't 1.0 here, I think this is because this isn't really a reversible operation

	Point posWFixed = Point(posResult.m_x, posResult.m_y, posResult.m_z);
	return posWFixed * MatObjectToWorld();
}
