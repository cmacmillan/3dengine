#pragma once

#include "object.h"
#include "node.h"

struct SNode3D : SNode // node3D
{
	typedef SNode super;
	SNode3DHandle HNode3D() { return (SNode3DHandle) m_nHandle; }

	SNode3D(SNodeHandle hNodeParent);

	Transform m_transformLocal;
};
