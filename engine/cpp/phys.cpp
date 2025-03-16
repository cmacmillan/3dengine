#include "phys.h"
#include "engine.h"

SPhysCube::SPhysCube(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
}

const Point g_posBoxLow = Point(-1.0f, -1.0f, -1.0f);
const Point g_posBoxHigh = Point(1.0f, 1.0f, 1.0f);

void SPhysCube::UpdateSelfAndChildTransformCache()
{
	super::UpdateSelfAndChildTransformCache();

	m_vecNonuniformScale = g_vecOne;
	m_gUniformScale = 1.0f;
	m_matPhys = g_matIdentity;

	// BB I don't think translation actually is a problem for nonuniform scales, only rotation
	//  We'd have to adjust the rest of the scheme slightly to allow for that tho

	bool fRotationOrTranslationEncountered = false;
	SNode3D * pNode3d = this;
	while (pNode3d != nullptr)
	{
		Vector vecScaleLocal = pNode3d->VecScaleLocal();
		if (!fRotationOrTranslationEncountered)
		{
			m_vecNonuniformScale = VecComponentwiseMultiply(m_vecNonuniformScale, vecScaleLocal);

			if (!pNode3d->QuatLocal().FIsIdentity() || !FIsNear(pNode3d->PosLocal(), g_posZero))
			{
				fRotationOrTranslationEncountered = true;
				m_matPhys = MatRotate(pNode3d->QuatLocal()) * MatTranslate(pNode3d->PosLocal());
			}
		}
		else
		{
			ASSERT(vecScaleLocal.X() == vecScaleLocal.Y() && vecScaleLocal.Y() == vecScaleLocal.Z());
			m_gUniformScale *= vecScaleLocal.X();

			m_matPhys = m_matPhys * pNode3d->MatObjectToParent();
		}

		pNode3d = pNode3d->PNode3DParent();
	}

	m_matPhysInverse = MatInverse(m_matPhys);

	ASSERT(FIsNear(SLength(m_matPhys.VecX()), m_gUniformScale));
	ASSERT(FIsNear(SLength(m_matPhys.VecY()), m_gUniformScale));
	ASSERT(FIsNear(SLength(m_matPhys.VecZ()), m_gUniformScale));
}

void SPhysCube::Update()
{
	// BB Every phys cube being a full entity with an update function is somewhat disgusting
	//  ... Or every drawnode, for that matter

	TWEAKABLE bool s_fDrawPhysCubes = false;

	if (s_fDrawPhysCubes)
	{
		g_game.DebugDrawCube(MatObjectToWorld());
	}
}

void IntersectRayWithAllPhys(Point posOrigin, Vector normalDirection, std::vector<SIntersection> * paryIntersection)
{
	for (SObject * pObj : g_objman.m_mpTypekAryPObj[TYPEK_PhysCube])
	{
		SPhysCube & physcube = *static_cast<SPhysCube *>(pObj);

		ASSERT(SLength(physcube.VecScaleLocal()) > 0.0000001f);

		Mat matObjToWorld = physcube.MatObjectToWorld();
		Mat matWorldToObj = MatInverse(matObjToWorld);

		Point posOriginLocal = posOrigin * matWorldToObj;
		Vector vecRayLocal =  normalDirection * matWorldToObj;

		// https://en.wikipedia.org/wiki/Slab_method

		Vector dPosLow = g_posBoxLow - posOriginLocal;
		Vector dPosHigh = g_posBoxHigh - posOriginLocal;

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

bool FRaycast(Point posOrigin, Vector normalDirection, Point * pPosIntersection)
{
	// TODO make this better

	std::vector<SIntersection> aryIntersection;
	IntersectRayWithAllPhys(posOrigin, normalDirection, &aryIntersection);

	if (aryIntersection.size() == 0)
		return false;

	float sMin = FLT_MAX;
	Point posBest = g_posZero;
	for (const SIntersection & intersection : aryIntersection)
	{
		if (intersection.m_s < sMin)
		{
			sMin = intersection.m_s;
			posBest = intersection.m_pos;
		}
	}
	ASSERT(sMin != FLT_MAX);
	*pPosIntersection = posBest;
	return true;
}

SDynSphere::SDynSphere(SNode * pNodeParent, const std::string & strName, TYPEK typek) : super(pNodeParent, strName, typek)
{
}

void SDynSphere::Update()
{
	super::Update();

	// NOTE written assuming the sphere has no parent

	// BB This should be on a fixed timestep for a variety of reasons
	//  To make phys behave consistently at different framerates
	//  To avoid becoming a bottleneck for high framerates
	//  To avoid precision issues at high framerates (due to small dT)

	Point posWorld = PosWorld();

	if (m_fFirstUpdate)
	{
		m_fFirstUpdate = false;
		m_dTPrev = 1.0f / 60.0f;
		m_posPrev = posWorld;
	}

	float dT = g_game.m_dT;
	ASSERT(!FIsNear(dT, 0.0f));

	// Verlet integration

	// Accumulate Forces

	Vector vecForces = g_vecZero;

	TWEAKABLE float s_gGravity = -30.0f;//-9.8f;
	Vector vecGravity = Vector(0.0f, 0.0f, s_gGravity);
	vecForces = vecForces + vecGravity;

	// Update position based on forces & velocity

	Vector vPrev = (posWorld - m_posPrev)/m_dTPrev;
	vPrev = vPrev + vecForces * dT;
	Vector dPosFromVelocity = vPrev * dT;
	Point posNew = posWorld + dPosFromVelocity;
	Point posNewOriginal = posNew;
	m_posPrev = posWorld;
	m_dTPrev = dT;

	// Apply constraints & solve collisions

	ASSERT(m_transformLocal.m_vecScale.X() == m_transformLocal.m_vecScale.Y() && m_transformLocal.m_vecScale.X() == m_transformLocal.m_vecScale.Z()); // ensure uniform scale
	float sRadius = m_transformLocal.m_vecScale.X();

	bool m_fCollision = false;

	for (SObject * pObj : g_objman.m_mpTypekAryPObj[TYPEK_PhysCube])
	{
		SPhysCube & physcube = *static_cast<SPhysCube *>(pObj);

		Point posNewLocal = posNew * physcube.m_matPhysInverse;

		Vector vecSign = Vector(GSign(posNewLocal.X()), GSign(posNewLocal.Y()), GSign(posNewLocal.Z()));

		Vector dPosToEdge = VecComponentwiseMultiply(vecSign, Vector(posNewLocal)) - physcube.m_vecNonuniformScale;

		float sMax = GMax(GMax(dPosToEdge.X(), dPosToEdge.Y()), dPosToEdge.Z());

		if (sMax > sRadius)
			continue; // no collision

		m_fCollision = true;

		Vector vecPush;
		if (dPosToEdge.X() > dPosToEdge.Y() && dPosToEdge.X() > dPosToEdge.Z())
		{
			// push out along x

			vecPush = Vector(vecSign.X() * (sRadius - dPosToEdge.X()), 0.0f, 0.0f);
		}
		else if (dPosToEdge.Y() > dPosToEdge.Z())
		{
			// push out along y

			vecPush = Vector(0.0f, vecSign.X() * (sRadius - dPosToEdge.Y()), 0.0f);
		}
		else
		{
			// push out along z

			vecPush = Vector(0.0f, 0.0f, vecSign.Z() * (sRadius - dPosToEdge.Z()));
		}

		posNew = posNew + vecPush * physcube.m_matPhys;
	}

	if (m_fCollision)
	{
		TWEAKABLE float s_rBounciness = 0.7f;
		Vector dPosCumulative = posNew - posNewOriginal;
		float sDPosCumulative = SLength(dPosCumulative);
		if (sDPosCumulative > 0.0f)
		{
			Vector normalCumulative = dPosCumulative / sDPosCumulative;
			Vector dPosAligned = VecProjectOnNormal(dPosFromVelocity, normalCumulative);
			Vector dPosPerp = dPosFromVelocity - dPosAligned;

			// Proportional to the normal force

			TWEAKABLE float s_gFriction = 0.01f;
			float gNormalForce = 2.0f * SLength(dPosAligned) / dT;

			Vector dPosFinal = -dPosAligned * s_rBounciness + dPosPerp * GMin(1.0f, 1.0f - gNormalForce * s_gFriction);

			m_posPrev = posNew - dPosFinal;
		}
	}

	ASSERT(!posNew.m_vec.FHasNans());

	SetPosWorld(posNew);
}






/////////////////////////////



// GJK

// The general idea here is we can use the gjk algorithm to sweep paths, and binary search for a time of intersection
// This doesn't account for rotation of the objects over this timestep
// The time of intersection will be considered to have happened when within some epsilon of the surface, 
// perhaps actually when within some epsilon +/- some smaller epsilon, since we don't likely want
// points that are right at the surface for numerical stability reasons
// e.g. we'll accept a distance which is .01-.02 away from the surface
// not right at the surface since the normal delta will be unnormalizable junk
// So we're gonna want a sweep path function, which perhaps can take in a const reference context struct containing the relevant things
// Only one moving object for now A. We minkowski sum it with a line segment L, and subtract other object B The two shapes intersect if (A+L)-B = 0
// If I know rough distance, I can potentially move in a raymarch-y way
// We need furthest along in some direction vector
// support = max point in a given direction
// support(a) - support(b) gives us a point in the 3d minkoski space, kinda weird since we're subtracting points, but it's a difference vector
// casey's condition is "go as far as possible in the given direction"
// These other folks use once the

#if 1
void DoStuff()
{
#if 0
	Vector normalSupport = g_vecXAxis;

	SDynSphere * pDynsphere;
	SPhysCube * pPhyscube;
	Vector dPosSweep;

	SFixArray<Point, 4> aryPos;

	// BB using local cube scale
	// BB using local sphere x scale

	aryPos.Append(PosSupportSweptSphere(pDynsphere->PosWorld(), pDynsphere->VecScaleLocal().X(), dPosSweep, normalSupport) - PosSupportBox(pPhyscube->PosWorld(), pPhyscube->VecScaleLocal(), -normalSupport));

	normalSupport = -Vector(aryPos[0]);

	float sMin = FLT_MAX;
#endif
}

Point PosSupportSweptSphere(Point posSphere, float sRadius, Vector dPosSweep, Vector normalSupport)
{
	float gDot = GDot(normalSupport, dPosSweep);
	Point posTest = (gDot > 0.0f) ? posSphere + dPosSweep : posSphere;
	return posTest + sRadius * normalSupport;
}

Point PosSupportBox(const Mat & matPhys, Vector vecNonuniformScale,  Vector normalSupport)
{
	// BB constructing vectors here

	// NOTE that the length of these vectors is whatever the uniform scale is

	Vector vecX = matPhys.m_aVec[0];
	Vector vecY = matPhys.m_aVec[1];
	Vector vecZ = matPhys.m_aVec[2];

	float gDotX = GDot(vecX, normalSupport);
	float gDotY = GDot(vecY, normalSupport);
	float gDotZ = GDot(vecZ, normalSupport);

	// Assuming box ranges from -1 to 1

	Point pos = matPhys.m_aVec[3];
	pos += (gDotX > 0.0f) ? vecX * vecNonuniformScale.X() : -vecX * vecNonuniformScale.X();
	pos += (gDotY > 0.0f) ? vecY * vecNonuniformScale.Y() : -vecY * vecNonuniformScale.Y();
	pos += (gDotZ > 0.0f) ? vecZ * vecNonuniformScale.Z() : -vecZ * vecNonuniformScale.Z();

	return pos;
}

void TestGjk()
{
	TWEAKABLE int s_iPhyscubeTest = 0;
	TWEAKABLE int s_iDynsphereTest = 0;
	auto & aryPhyscube = g_objman.m_mpTypekAryPObj[TYPEK_PhysCube];
	auto & aryDynsphere = g_objman.m_mpTypekAryPObj[TYPEK_DynSphere];
	SDynSphere * pDynsphere = (SDynSphere *) aryDynsphere[s_iDynsphereTest];
	SPhysCube * pPhyscube = (SPhysCube *)aryPhyscube[s_iPhyscubeTest];
	g_game.DebugDrawCube(pPhyscube->MatObjectToWorld());
	g_game.DebugDrawSphere(pDynsphere->PosWorld(), pDynsphere->VecScaleLocal().X());

	TWEAKABLE Vector s_vecSupport = g_vecXAxis;
	Vector normalSupport = VecNormalize(s_vecSupport);

	g_game.DebugDrawArrow(pPhyscube->PosWorld(), normalSupport*4.0f);
	g_game.DebugDrawSphere(PosSupportBox(pPhyscube->m_matPhys, pPhyscube->m_vecNonuniformScale, normalSupport));
	DoNothing();
}
#endif









