#pragma once

#include "object.h"
#include "node3d.h"

struct SCamera3D : SNode3D // camera3D
{
	typedef SNode3D super;
	SCamera3DHandle HCamera3D() { return (SCamera3DHandle) m_nHandle; }

	SCamera3D(SNodeHandle hNodeParent, const std::string & strName, float radFovHorizontal, float xNearClip, float xFarClip);

	float m_radFovHorizontal = -1;
	float m_xNearClip = -1;
	float m_xFarClip = -1;
};
