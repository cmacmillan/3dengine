#include "camera3d.h"
#include "engine.h"

SCamera3D::SCamera3D(SNode * pNodeParent,  const std::string & strName, float radFovHorizontal, float xNearClip, float xFarClip, TYPEK typek) :
	super(pNodeParent, strName, typek),
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

	float w;
	if (m_fOrthographic)
		w = (m_xFarClip-m_xNearClip)/m_xFarClip;
	else
		w = GMapRange(1.0f, 0.0f, m_xNearClip, m_xFarClip, posNdc.m_vec.m_z);

	// Multiplying by what ends up in the w component

	float4 posResult = (posNdc.m_vec * w) * matClipToCamera;

	// NOTE W isn't 1.0 here, I think this is because this isn't really a reversible operation

	Point posWFixed = Point(posResult.m_x, posResult.m_y, posResult.m_z);
	return posWFixed * MatObjectToWorld();
}

Point SCamera3D::PosNdcFromPosWindow(float2 posWindow, float xDepthWorld)
{
	float2 vecWinSize = g_game.VecWinSize();
	float xNdc = GMapRange(0.0f, vecWinSize.m_x, -1.0f, 1.0f, posWindow.m_x);
	float yNdc = GMapRange(0.0f, vecWinSize.m_y, 1.0f, -1.0f, posWindow.m_y);
	return Point(xNdc, yNdc, GMapRange(m_xNearClip, m_xFarClip, 1.0f, 0.0f, xDepthWorld));
}

SFrustum SCamera3D::FrustumCompute()
{
	// NOTE these coordinates are somewhat weird since it's a mapping of NDC to world space
	//  E.g. inverse z becomes x

	SFrustum frustum;
	frustum.m_posMin = PosWorldFromPosNdc(Point(1.0f, -1.0f, 1.0f));
	frustum.m_posMinYzMaxX = PosWorldFromPosNdc(Point(1.0f, -1.0f, 0.0f));
	frustum.m_posMinXzMaxY = PosWorldFromPosNdc(Point(-1.0f, -1.0f, 1.0f));
	frustum.m_posMinXyMaxZ = PosWorldFromPosNdc(Point(1.0f, 1.0f, 1.0f));
	frustum.m_posMinXMaxYz = PosWorldFromPosNdc(Point(-1.0f, 1.0f, 1.0f));
	frustum.m_posMinYMaxXz = PosWorldFromPosNdc(Point(1.0f, 1.0f, 0.0f));
	frustum.m_posMinZMaxXy = PosWorldFromPosNdc(Point(-1.0f, -1.0f, 0.0f));
	frustum.m_posMax = PosWorldFromPosNdc(Point(-1.0f, 1.0f, 0.0f));
	return frustum;
}
