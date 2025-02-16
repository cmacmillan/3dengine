#include "phys.h"
#include "engine.h"

SPhysCube::SPhysCube(SNodeHandle hNodeParent, const std::string & strName, TYPEK typek) : super(hNodeParent, strName, typek)
{
}

void IntersectRayWithAllPhys(Point posOrigin, Vector normalDirection, std::vector<SIntersection> * paryIntersection)
{
	for (SObject * pObj : g_objman.m_mpTypekAryPObj[TYPEK_PhysCube])
	{
		SPhysCube & physcube = *static_cast<SPhysCube *>(pObj);

		ASSERT(SLength(physcube.VecScaleLocal()) > 0.0000001f);

		Point posBoxLow = Point(-1.0f, -1.0f, -1.0f);
		Point posBoxHigh = Point(1.0f, 1.0f, 1.0f);

		Mat matObjToWorld = physcube.MatObjectToWorld();
		Mat matWorldToObj = MatInverse(matObjToWorld);

		Point posOriginLocal = posOrigin * matWorldToObj;
		Vector vecRayLocal =  normalDirection * matWorldToObj;

		// https://en.wikipedia.org/wiki/Slab_method

		Vector dPosLow = posBoxLow - posOriginLocal;
		Vector dPosHigh = posBoxHigh - posOriginLocal;

		// NOTE vecRayLocal might be zero here, in that case this division will produce a nan.
		//  We handle that below.

		Vector vecLow = VecComponentwiseDivide(dPosLow, vecRayLocal);
		Vector vecHigh = VecComponentwiseDivide(dPosHigh, vecRayLocal);

		Vector vecClose = VecComponentwiseMin(vecLow, vecHigh);
		Vector vecFar = VecComponentwiseMax(vecLow, vecHigh);

		// Since vec ray local is guarenteed to not be zero length, at least one component is non-zero

		float gClose = -FLT_MAX;
		float gFar = FLT_MAX;
		if (vecRayLocal.X() != 0.0f)
		{
			gClose = GMax(gClose, vecClose.X());
			gFar = GMin(gFar, vecFar.X());
		}

		if (vecRayLocal.Y() != 0.0f)
		{
			gClose = GMax(gClose, vecClose.Y());
			gFar = GMin(gFar, vecFar.Y());
		}

		if (vecRayLocal.Z() != 0.0f)
		{
			gClose = GMax(gClose, vecClose.Z());
			gFar = GMin(gFar, vecFar.Z());
		}

		if (gFar < gClose)
			continue; // no intersection

		if (gClose < 0.0f)
			continue;

		Point posHitCloseWorld = (posOriginLocal + gClose * vecRayLocal) * matObjToWorld;
		Point posHitFarWorld = (posOriginLocal + gFar * vecRayLocal) * matObjToWorld;

		paryIntersection->push_back({ posHitCloseWorld, SLength(posHitCloseWorld - posOrigin), &physcube });
		paryIntersection->push_back({ posHitFarWorld, SLength(posHitFarWorld - posOrigin), &physcube});
	}
}
