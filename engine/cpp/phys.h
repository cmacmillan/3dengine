#pragma once

#include "object.h"
#include "node3d.h"

struct SPhysCube : SNode3D // physcube
{
	typedef SNode3D super;
	SPhysCubeHandle HPhyscube() { return (SPhysCubeHandle) m_nHandle; }

	SPhysCube(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek = TYPEK_PhysCube);
};

struct SIntersection // intersection
{
	Point m_pos;
	float m_s;
	SPhysCube * m_pPhyscube;
};

void IntersectRayWithAllPhys(Point posOrigin, Vector normalDirection, std::vector<SIntersection> * paryIntersection);
