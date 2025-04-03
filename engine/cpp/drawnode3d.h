#pragma once

#include "object.h"
#include "node3d.h"
#include "color.h"

struct SDrawNodeRenderConstants
{
	void FillOut(const Mat & matObjectToWorld, const Mat & matWorldToClip, const Mat & matWorldToCamera, const Mat & matWorldToObject, SRgba rgba = SRgba(1.0f, 1.0f, 1.0f, 1.0f));

	Mat		m_matMVP;
	Mat		m_matVP;
	Mat		m_matV;
	Mat		m_matObjectToWorld;
	Mat		m_matWorldToObject;
	Mat		m_matObjectToWorldInverseTranspose;
	SRgba	m_rgba;
};

struct SDrawNode3D : SNode3D // drawnode3D
{
	typedef SNode3D super;
	SDrawNode3DHandle HDrawnode3D() { return (SDrawNode3DHandle) m_h; }

	SDrawNode3D(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_DrawNode3D);

	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};
