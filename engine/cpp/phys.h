#pragma once

#include "object.h"
#include "node3d.h"

struct SPhysCube : SNode3D // physcube
{
	typedef SNode3D super;
	SPhysCubeHandle HPhyscube() { return (SPhysCubeHandle) m_h; }

	SPhysCube(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_PhysCube);
	void UpdateSelfAndChildTransformCache() override;
	void Update() override;

	Vector m_vecNonuniformScale;	// The non-uniform scale component which is allowed to live at the bottom of the hierarchy
	float m_gUniformScale;			// The uniform scale which is a part of matPhys
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
	SDynSphereHandle HPhyscube() { return (SDynSphereHandle) m_h; }

	SDynSphere(SNode * pNodeParent, const std::string & strName, TYPEK typek = TYPEK_DynSphere);

	void Update() override;

	float SRadius()
		{ return VecScaleLocal().X(); }

	bool m_fFirstUpdate = true;
	Point m_posPrev;
	float m_dTPrev;
};

struct IGjk
{
	virtual Point PosSupport(const Vector & normalSupport) = 0;
};

enum GJKRES
{
	GJKRES_NoHit = 0,
	GJKRES_Hit = 1,
	GJKRES_Panic = 2,
};

GJKRES GjkresSweep(IGjk * pGjkSweeper, IGjk * pGjkStatic, Point * pPosSweeperEnd, Point * pPosClosestOnSweeper, Point * pPosClosestOnStatic);

struct SGjkIcosphere : IGjk
{
					SGjkIcosphere(const Point & pos, float sRadius) :
						m_pos(pos),
						m_sRadius(sRadius)
					{ }

	Point PosSupport(const Vector & normalSupport) override;

	Point m_pos;
	float m_sRadius;
};

struct SGjkSphere : IGjk
{
					SGjkSphere(const Point & pos, float sRadius) :
						m_pos(pos),
						m_sRadius(sRadius)
					{ }

	Point PosSupport(const Vector & normalSupport) override;

	Point m_pos;
	float m_sRadius;
};

struct SGjkBox : IGjk
{
					SGjkBox(const Mat & matPhys, const Vector & vecNonuniformScale) :
						m_matPhys(matPhys),
						m_vecNonuniformScale(vecNonuniformScale)
					{ }

	Point PosSupport(const Vector & normalSupport) override;

	Mat m_matPhys;
	Vector m_vecNonuniformScale;
};

void TestGjk(); 