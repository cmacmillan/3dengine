#pragma once

#include "object.h"
#include "node3d.h"
#include "render.h"

struct SCamera3D : SNode3D // camera3D
{
	typedef SNode3D super;
	SCamera3DHandle HCamera3D() { return (SCamera3DHandle) m_h; }

	SCamera3D(SNode * pNodeParent, const std::string & strName, float radFovHorizontal, float xNearClip, float xFarClip, TYPEK typek = TYPEK_Camera3D);

	void SetOrthographic(float gScaleOrthographic);

	Mat MatCameraToClip();
	Mat MatWorldToClip();
	Mat MatClipToCamera();
	Mat MatClipToWorld();

	Point PosWorldFromPosNdc(Point posNdc);
	Point PosNdcFromPosWindow(float2 posWindow, float xDepthWorld);

	SFrustum FrustumCompute();

	float m_radFovHorizontal = -1;
	float m_xNearClip = -1;
	float m_xFarClip = -1;

	bool m_fOrthographic = false;
	float m_gScaleOrthographic = -1.0f;
};
