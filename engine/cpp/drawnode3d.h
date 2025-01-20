#pragma once

#include "object.h"
#include "node3d.h"

struct SDrawNodeRenderConstants
{
	Mat m_matMVP;
	Mat m_matObjectToWorld;
};

struct SDrawNode3D : SNode3D // drawnode3D
{
	typedef SNode3D super;
	SDrawNode3DHandle HDrawnode3D() { return (SDrawNode3DHandle) m_nHandle; }

	SDrawNode3D(SNodeHandle hNodeParent, const std::string & strName);

	SMaterialHandle m_hMaterial = -1;
	SMesh3DHandle m_hMesh = -1;
};
