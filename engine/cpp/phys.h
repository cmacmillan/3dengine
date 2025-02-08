#pragma once

#include "object.h"
#include "node3d.h"

struct SPhysCube : SNode3D // physcube
{
	typedef SNode3D super;
	SPhysCubeHandle HPhyscube() { return (SPhysCubeHandle) m_nHandle; }

	SPhysCube(SNodeHandle hNodeParent, const std::string & strName);
};
