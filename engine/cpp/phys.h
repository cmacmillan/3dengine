#pragma once

#include "object.h"
#include "node3d.h"

struct SPhysCube : SNode3D // physcube
{
	typedef SNode3D super;
	SPhysCubeHandle HPhyscube() { return (SPhysCubeHandle) m_nHandle; }

	SPhysCube(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_PhysCube);
	void UpdateSelfAndChildTransformCache() override;
	void Update() override;

	Vector m_vecNonuniformScale;	// The non-uniform scale component which is allowed to live at the bottom of the hierarchy
	float m_gUniformScale;			// The scale we'd need to incorporate if we wanted to actually get world-space distance
	Mat m_matPhys;					// Excludes any non-uniform scale at the bottom of the hierarchy
	Mat m_matPhysInverse;			//  ...
};

struct SIntersection // intersection
{
	Point m_pos;
	float m_s;
	SPhysCube * m_pPhyscube;
};

void IntersectRayWithAllPhys(Point posOrigin, Vector normalDirection, std::vector<SIntersection> * paryIntersection);
bool FRaycast(Point posOrigin, Vector normalDirection, Point * pPosIntersection);

struct SDynSphere : SNode3D // dynsphere 
{
	typedef SNode3D super;
	SDynSphereHandle HPhyscube() { return (SDynSphereHandle) m_nHandle; }

	SDynSphere(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_DynSphere);

	void Update() override;

	bool m_fFirstUpdate = true;
	Point m_posPrev;
	float m_dTPrev;
};
