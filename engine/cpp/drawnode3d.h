#pragma once

#include "object.h"
#include "node3d.h"

struct SDrawNodeRenderConstants
{
	void FillOut(Mat matObjectToWorld, Mat matWorldToClip);

	Mat m_matMVP;
	Mat m_matObjectToWorld;
	Mat m_matObjectToWorldInverseTranspose;
};

struct SDrawNode3D : SNode3D // drawnode3D
{
	typedef SNode3D super;
	SDrawNode3DHandle HDrawnode3D() { return (SDrawNode3DHandle) m_nHandle; }

	SDrawNode3D(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek = TYPEK_DrawNode3D);

	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};
