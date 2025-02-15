#include "phys.h"
#include "engine.h"

SPhysCube::SPhysCube(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek) : super(hNodeParent, strName, typek)
{
}

void IntersectRayWithAllPhys(Point posOrigin, Vector normalDirection, std::vector<SIntersection> * paryIntersection)
{
	for (const SObject * pObj : g_objman.m_mpTypekAryPObj[TYPEK_PhysCube])
	{
		const SPhysCube & physcube = *static_cast<const SPhysCube *>(pObj);

		Point posBoxLow = Point(-1.0f, -1.0f, -1.0f);
		Point posBoxHigh = Point(1.0f, 1.0f, 1.0f);

		Mat matObjToWorld = physcube.MatObjectToWorld();
		Mat matWorldToObj = MatInverse(matObjToWorld);

		Point posOriginLocal = posOrigin * matWorldToObj;
		Vector vecRayLocal =  normalDirection * matWorldToObj;

		// https://en.wikipedia.org/wiki/Slab_method

		Vector vecLow = VecComponentwiseDivide(posBoxLow - posOriginLocal, vecRayLocal);
		Vector vecHigh = VecComponentwiseDivide(posBoxHigh - posOriginLocal, vecRayLocal);

		Vector vecClose = VecComponentwiseMin(vecLow, vecHigh);
		Vector vecFar = VecComponentwiseMax(vecLow, vecHigh);

		float gClose = GMaxInVec(vecClose);
		float gFar = GMinInVec(vecFar);

		if (gFar > gClose)
			continue; // no intersection

		Point posHitCloseWorld = (posOriginLocal + gClose * vecRayLocal) * matObjToWorld;
		Point posHitFarWorld = (posOriginLocal + gFar * vecRayLocal) * matObjToWorld;

		paryIntersection->push_back({ posHitCloseWorld, SLength(posHitCloseWorld - posOrigin) });
		paryIntersection->push_back({ posHitFarWorld, SLength(posHitFarWorld - posOrigin) });
	}
}
