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

// Eventually we're gonna compute a time of impact for each box, any times of impact that are within some epsilon should be considered to have happened at the same time

struct SMink
{
	Point m_posMink;
	Point m_posSrcA;
	Point m_posSrcB;
};

bool FHandleTriangle(SMink mink0, SMink mink1, SMink mink2, Vector * pNormalOutward, Vector * pNormalSupport, SFixArray<SMink, 4> * paryMink, Point * pPos0, Point * pPos1)
{
	Point pos0 = mink0.m_posMink;
	Point pos1 = mink1.m_posMink;
	Point pos2 = mink2.m_posMink;

	// Handle optional "outward" vector used in tetrahedron case

	if (pNormalOutward && GDot(*pNormalOutward, -pos0) < 0.0f)
		return false;

	Vector normalTriangle = VecNormalizeSafe(VecCross(pos1 - pos0, pos2 - pos0));
	Vector normal01 = VecCross(normalTriangle, pos1 - pos0);
	Vector normal12 = VecCross(normalTriangle, pos2 - pos1);
	Vector normal02 = VecCross(normalTriangle, pos2 - pos0);

	// Flip normals to ensure they point outward

	if (GDot(normal01, pos2 - pos0) > 0.0f)
	{
		normal01 = -normal01;
	}

	if (GDot(normal12, pos0 - pos1) > 0.0f)
	{
		normal12 = -normal12;
	}

	if (GDot(normal02, pos1 - pos0) > 0.0f)
	{
		normal02 = -normal02;
	}

	bool fInTri =	GDot(normal01, -pos0) < 0.0f &&
					GDot(normal12, -pos1) < 0.0f &&
					GDot(normal02, -pos0) < 0.0f;
	if (fInTri)
	{
		*pNormalSupport = VecNormalizeElse(VecProjectOnNormal(-pos0, normalTriangle), g_vecXAxis);
		paryMink->Empty();
		paryMink->Append(mink0);
		paryMink->Append(mink1);
		paryMink->Append(mink2);

		// Barycentric coordinates (derived in mathematica)

		Point posOrigin = VecProjectOnTangent(-pos0,normalTriangle)+pos0;

		float gDenom = 
			+ pos0.Z() * pos1.Y() * pos2.X()
			- pos0.Y() * pos1.Z() * pos2.X() 
			- pos0.Z() * pos1.X() * pos2.Y() 
			+ pos0.X() * pos1.Z() * pos2.Y() 
			+ pos0.Y() * pos1.X() * pos2.Z() 
			- pos0.X() * pos1.Y() * pos2.Z();
		ASSERT(!isnan(gDenom) && gDenom != 0.0f);

		float gTNum =
			- pos1.Z() * pos2.Y() * posOrigin.X()
			+ pos1.Y() * pos2.Z() * posOrigin.X()
			+ pos1.Z() * pos2.X() * posOrigin.Y()
			- pos1.X() * pos2.Z() * posOrigin.Y()
			- pos1.Y() * pos2.X() * posOrigin.Z()
			+ pos1.X() * pos2.Y() * posOrigin.Z();

		float gUNum =
			+ pos0.Z() * pos2.Y() * posOrigin.X()
			- pos0.Y() * pos2.Z() * posOrigin.X()
			- pos0.Z() * pos2.X() * posOrigin.Y()
			+ pos0.X() * pos2.Z() * posOrigin.Y()
			+ pos0.Y() * pos2.X() * posOrigin.Z()
			- pos0.X() * pos2.Y() * posOrigin.Z();

		float gVNum =
			- pos0.Z() * pos1.Y() * posOrigin.X()
			+ pos0.Y() * pos1.Z() * posOrigin.X()
			+ pos0.Z() * pos1.X() * posOrigin.Y()
			- pos0.X() * pos1.Z() * posOrigin.Y()
			- pos0.Y() * pos1.X() * posOrigin.Z()
			+ pos0.X() * pos1.Y() * posOrigin.Z();

		ASSERT(!isnan(gTNum) && !isnan(gUNum) && !isnan(gVNum));

		float gT = -gTNum/gDenom;
		float gU = -gUNum/gDenom;
		float gV = -gVNum/gDenom;

		*pPos0 = Vector(mink0.m_posSrcA) * gT + Vector(mink1.m_posSrcA) * gU + Vector(mink2.m_posSrcA) * gV;
		*pPos1 = Vector(mink0.m_posSrcB) * gT + Vector(mink1.m_posSrcB) * gU + Vector(mink2.m_posSrcB) * gV;
	}

	return fInTri;
}

int IOriginInLineSegment(Point pos0, Point pos1, float * pS = nullptr)
{
	Vector dPos = pos1 - pos0;
	float s = SLength(dPos);
	Vector normal = dPos / s;
	float gDot = GDot(-Vector(pos0)/s, normal);
	if (gDot > 0.0f && gDot < 1.0f)
	{
		// In segment
		if (pS)
		{
			*pS = SLength(pos0 + gDot * dPos);
		}
		return -1;
	}
	else if (gDot < 0.0f)
	{
		// Point 0
		return 0;
	}
	else
	{
		// Point 1
		return 1;
	}
}

void SetSimplexPoint(SMink mink, Vector * pNormalSupport, SFixArray<SMink, 4> * paryMink, Point * pPos0, Point * pPos1)
{
	paryMink->Empty();
	paryMink->Append(mink);
	*pPos0 = mink.m_posSrcA;
	*pPos1 = mink.m_posSrcB;
	*pNormalSupport = VecNormalizeSafe(-mink.m_posMink);
}

void SetSimplexLineSegment(SMink mink0, SMink mink1, Vector * pNormalSupport, SFixArray<SMink, 4> * paryMink, Point * pPos0, Point * pPos1)
{
	paryMink->Empty();
	paryMink->Append(mink0);
	paryMink->Append(mink1);
	Vector dPos = mink1.m_posMink - mink0.m_posMink;
	float s = SLength(dPos);
	Vector normal = dPos / s;
	float u = GDot(-mink0.m_posMink, normal)/s;
	ASSERT(u >= 0.0f && u <= 1.0f);
	*pPos0 = PosLerp(mink0.m_posSrcA, mink1.m_posSrcA, u);
	*pPos1 = PosLerp(mink0.m_posSrcB, mink1.m_posSrcB, u);
	*pNormalSupport = VecNormalizeSafe(VecProjectOnTangent(-mink0.m_posMink, normal));
}

bool FSimplex(Vector * pNormalSupport, SFixArray<SMink, 4> * paryMink, Point * pPos0, Point * pPos1)
{
	switch (paryMink->m_c)
	{
	case 2:
		{
			SMink mink0= (*paryMink)[0];
			SMink mink1 = (*paryMink)[1];
			switch (IOriginInLineSegment(mink0.m_posMink, mink1.m_posMink))
			{
				case -1:
					SetSimplexLineSegment(mink0, mink1, pNormalSupport, paryMink, pPos0, pPos1);
					break;

				case 0:
					SetSimplexPoint(mink0, pNormalSupport, paryMink, pPos0, pPos1);
					break;

				case 1:
					SetSimplexPoint(mink1, pNormalSupport, paryMink, pPos0, pPos1);
					break;

				default:
					ASSERT(false);
					break;
			}
			return false;
		}
		break;
	case 3:
		{
			SMink mink0 = (*paryMink)[0];
			SMink mink1 = (*paryMink)[1];
			SMink mink2 = (*paryMink)[2];
			if (FHandleTriangle(mink0, mink1, mink2, nullptr, pNormalSupport, paryMink, pPos0, pPos1))
				return false;

			// BB a bit gross

			const int c = 3;
			SMink aMink0[c] = { mink0, mink1, mink0 };
			SMink aMink1[c] = { mink1, mink2, mink2 };
			int aiOrigin[c];

			float sBest = FLT_MAX;
			float sPoint = FLT_MAX;
			int iBest = -1;
			int iPoint = -1;
			for (int i = 0; i < c; i++)
			{
				float sCur;
				aiOrigin[i] = IOriginInLineSegment(aMink0[i].m_posMink, aMink1[i].m_posMink, &sCur);
				if (aiOrigin[i] == -1)
				{
					if (sCur < sBest)
					{
						sBest = sCur;
						iBest = i;
					}
				}
				else
				{
					Point pos = (aiOrigin[i] == 0) ? aMink0[i].m_posMink : aMink1[i].m_posMink;
					sPoint = SLength(pos);
					iPoint = i;
				}
			}

			if (iBest != -1 && sPoint > sBest)
			{
				SetSimplexLineSegment(aMink0[iBest], aMink1[iBest], pNormalSupport, paryMink, pPos0, pPos1);
				return false;
			}

			SetSimplexPoint((aiOrigin[iPoint] == 0)? aMink0[iPoint] : aMink1[iPoint], pNormalSupport, paryMink, pPos0, pPos1);
			return false;
		}
		break;
	case 4:
		{
			SMink mink0 = (*paryMink)[0];
			SMink mink1 = (*paryMink)[1];
			SMink mink2 = (*paryMink)[2];
			SMink mink3 = (*paryMink)[3];

			Point pos0 = mink0.m_posMink;
			Point pos1 = mink1.m_posMink;
			Point pos2 = mink2.m_posMink;
			Point pos3 = mink3.m_posMink;

			Vector normal012 = VecCross(pos1 - pos0, pos2 - pos0);
			Vector normal123 = VecCross(pos2 - pos1, pos3 - pos1);
			Vector normal013 = VecCross(pos3 - pos0, pos1 - pos0);
			Vector normal023 = VecCross(pos2 - pos0, pos3 - pos0);

			// Flip normals to ensure they point outward

			if (GDot(normal012, pos3 - pos0) > 0.0f)
			{
				normal012 = -normal012;
			}

			if (GDot(normal123, pos0 - pos1) > 0.0f)
			{
				normal123 = -normal123;
			}

			if (GDot(normal013, pos2 - pos0) > 0.0f)
			{
				normal013 = -normal013;
			}

			if (GDot(normal023, pos1 - pos0) > 0.0f)
			{
				normal023 = -normal023;
			}

			if (GDot(normal012, -pos0) < 0.0f &&
				GDot(normal123, -pos1) < 0.0f &&
				GDot(normal013, -pos0) < 0.0f &&
				GDot(normal023, -pos0) < 0.0f)
			{
				// Inside tetrahedron

				return true;
			}

			if (FHandleTriangle(mink0, mink1, mink2, &normal012, pNormalSupport, paryMink, pPos0, pPos1))
				return false;

			if (FHandleTriangle(mink0, mink1, mink3, &normal013, pNormalSupport, paryMink, pPos0, pPos1))
				return false;

			if (FHandleTriangle(mink1, mink2, mink3, &normal123, pNormalSupport, paryMink, pPos0, pPos1))
				return false;

			if (FHandleTriangle(mink0, mink2, mink3, &normal023, pNormalSupport, paryMink, pPos0, pPos1))
				return false;

			// BB a bit gross

			const int c = 6;
			SMink aMink0[c] = { mink0, mink1, mink0, mink0, mink1, mink2 };
			SMink aMink1[c] = { mink1, mink2, mink2, mink3, mink3, mink3};
			int aiOrigin[c];

			float sBest = FLT_MAX;
			float sPoint = FLT_MAX;
			int iBest = -1;
			int iPoint = -1;
			for (int i = 0; i < c; i++)
			{
				float sCur;
				aiOrigin[i] = IOriginInLineSegment(aMink0[i].m_posMink, aMink1[i].m_posMink, &sCur);
				if (aiOrigin[i] == -1)
				{
					if (sCur < sBest)
					{
						sBest = sCur;
						iBest = i;
					}
				}
				else
				{
					Point pos = (aiOrigin[i] == 0) ? aMink0[i].m_posMink : aMink1[i].m_posMink;
					sPoint = SLength(pos);
					iPoint = i;
				}
			}

			if (iBest != -1 && sPoint > sBest)
			{
				SetSimplexLineSegment(aMink0[iBest], aMink1[iBest], pNormalSupport, paryMink, pPos0, pPos1);
				return false;
			}

			SetSimplexPoint((aiOrigin[iPoint] == 0)? aMink0[iPoint] : aMink1[iPoint], pNormalSupport, paryMink, pPos0, pPos1);
			return false;
		}
		break;

	default:
		ASSERT(false);
		return false;
	}
}

#define ENABLE_DEBUG_GJK 1

TWEAKABLE Point s_posMinkowskiOriginDebugDraw = Point(5.0f, 0.0f, 3.0f);
static int s_iDrawGjkDebug = 0;

// TODO need to figure out how this distance extrusion is gonna work

TWEAKABLE float s_sHitMin = .001f; // Any closer and we won't be able to produce a good normal, so just pretend this was a collision

Vector VecSweepBase(const Vector & dPosSweep, const Vector & normalSupport)
{
	float gDot = GDot(normalSupport, dPosSweep);
	return (gDot > 0.0f) ? dPosSweep : g_vecZero;
}

SMink MinkCompute(IGjk * pGjkSweep, IGjk * pGjkStatic, Vector dPosSweep, Vector normalSupport)
{
	Point posSrcA = pGjkSweep->PosSupport(normalSupport) + VecSweepBase(dPosSweep, normalSupport);
	Point posSrcB = pGjkStatic->PosSupport(-normalSupport);
	Point posMink = posSrcA - posSrcB;
	return { posMink, posSrcA, posSrcB };
}

GJKRES GjkresSweep(IGjk * pGjkSweeper, IGjk * pGjkStatic, Point * pPosSweeperEnd, Point * pPosClosestOnSweeper, Point * pPosClosestOnStatic)
{
	return GJKRES_Panic;
}

Point SGjkIcosphere::PosSupport(const Vector & normalSupport)
{
	Point aPosIcosphere[12] = { 
		Point(0, 0, 1), 
		Point(-0.7236f,-0.52572f,0.447215f),
		Point(0.276385,-0.85064,0.447215),
		Point(0.894425,0,0.447215),
		Point(0.276385,0.85064,0.447215),
		Point(-0.7236,0.52572,0.447215),
		Point(0.7236,0.52572,-0.447215),
		Point(0.7236,-0.52572,-0.447215),
		Point(-0.276385,-0.85064,-0.447215),
		Point(-0.894425,0,-0.447215),
		Point(-0.276385,0.85064,-0.447215),
		Point(0,0,-1)
	};

	float gDotBest = -FLT_MAX;
	Point posBest = g_posZero;
	{
		for (int iPos = 0; iPos < DIM(aPosIcosphere); iPos++)
		{
			Point pos = Vector(aPosIcosphere[iPos]) * m_sRadius + m_pos;
			float gDot = GDot(pos, normalSupport);
			if (gDot > gDotBest)
			{
				gDotBest = gDot;
				posBest = pos;
			}
		}
	}

	return posBest;
}

Point SGjkSphere::PosSupport(const Vector & normalSupport)
{
	return m_sRadius * normalSupport;
}

Point SGjkBox::PosSupport(const Vector & normalSupport)
{
	// BB constructing vectors here

	// NOTE that the length of these vectors is whatever the uniform scale is

	Vector vecX = m_matPhys.m_aVec[0];
	Vector vecY = m_matPhys.m_aVec[1];
	Vector vecZ = m_matPhys.m_aVec[2];

	float gDotX = GDot(vecX, normalSupport);
	float gDotY = GDot(vecY, normalSupport);
	float gDotZ = GDot(vecZ, normalSupport);

	// Assuming box ranges from -1 to 1

	Point pos = m_matPhys.m_aVec[3];
	pos += (gDotX > 0.0f) ? vecX * m_vecNonuniformScale.X() : -vecX * m_vecNonuniformScale.X();
	pos += (gDotY > 0.0f) ? vecY * m_vecNonuniformScale.Y() : -vecY * m_vecNonuniformScale.Y();
	pos += (gDotZ > 0.0f) ? vecZ * m_vecNonuniformScale.Z() : -vecZ * m_vecNonuniformScale.Z();

	return pos;
}

bool FGjk(IGjk * pGjkSweep, IGjk * pGjkStatic, Vector dPosSweep, Vector normalSupport, bool fDebug, float * pS)
{
	SFixArray<SMink, 4> aryMink;

	aryMink.Append(MinkCompute(pGjkSweep, pGjkStatic, dPosSweep, normalSupport));

	normalSupport = VecNormalizeSafe(-Vector(aryMink[0].m_posMink));

#if ENABLE_DEBUG_GJK
	SRgba aRgba[] = { g_rgbaRed, g_rgbaRed, g_rgbaOrange, g_rgbaYellow, g_rgbaPink };
	int iDraw = 0;
	if (fDebug)
	{
		g_game.PrintConsole(StrPrintf("iDraw:%i\n", s_iDrawGjkDebug));
	}
#endif

	float sMin = FLT_MAX;
	Point pos0Best;
	Point pos1Best;
	for (;;)
	{
		SMink mink = MinkCompute(pGjkSweep, pGjkStatic, dPosSweep, normalSupport);
		for (int i = 0; i < aryMink.m_c; i++)
		{
			if (FIsNear(aryMink[i].m_posMink, mink.m_posMink))
			{
#if ENABLE_DEBUG_GJK
				if (fDebug)
				{
					g_game.PrintConsole("early out!\n");
					g_game.DebugDrawSphere(pos0Best, .5f);
					g_game.DebugDrawSphere(pos1Best, .5f);
				}
#endif
				*pS = sMin;
				return false;
			}
		}

		aryMink.Append(mink);

#if ENABLE_DEBUG_GJK
		Vector vecPosArrow = g_vecZero;
		SRgba rgba = SRgba(0, 0, 0, 0);
		if (fDebug)
		{
			{
				if (iDraw == s_iDrawGjkDebug)
				{
					rgba = aRgba[aryMink.m_c];
					for (int i = 0; i < aryMink.m_c; i++)
					{
						vecPosArrow += aryMink[i].m_posMink + s_posMinkowskiOriginDebugDraw;
						g_game.DebugDrawSphere(aryMink[i].m_posMink + s_posMinkowskiOriginDebugDraw, .5f, 0.0f, rgba);
						for (int j = 0; j < aryMink.m_c; j++)
						{
							if (i == j)
								continue;
							g_game.DebugDrawLine(aryMink[i].m_posMink + s_posMinkowskiOriginDebugDraw, aryMink[j].m_posMink + s_posMinkowskiOriginDebugDraw, 0.0f, rgba);
						}
					}
					g_game.PrintConsole(StrPrintf("PointCount:%i\n", aryMink.m_c));
					vecPosArrow = vecPosArrow / aryMink.m_c;
				}
			}
		}
#endif

		float sCur;
		Point pos0;
		Point pos1;
		if (FSimplex(&normalSupport, &aryMink, &pos0, &pos1))
		{
			sCur = 0.0f;
		}
		else
		{
			sCur = SLength(pos1 - pos0);
		}

		if (sCur <= s_sHitMin)
		{
			// Hit!

			*pS = 0.0f;
			return true;
		}

		if (sCur >= sMin)
		{
			// We've stopped improving, miss!

#if ENABLE_DEBUG_GJK
			if (fDebug)
			{
				g_game.DebugDrawSphere(pos0Best, .5f);
				g_game.DebugDrawSphere(pos1Best, .5f);
			}
#endif
			*pS = sCur;
			return false;
		}

		sMin = sCur;
		pos0Best = pos0;
		pos1Best = pos1;

#if ENABLE_DEBUG_GJK
		if (fDebug)
		{
			if (iDraw == s_iDrawGjkDebug)
			{
				ASSERT(FIsNear(1.0f, SLength(normalSupport)));
				g_game.DebugDrawArrow(Point(vecPosArrow), normalSupport * 3.0f, .1f, 0.0f, rgba);
			}
			iDraw++;
		}
#endif
	}
}

void TestGjk(const Mat & matCubePhys, Vector vecNonuniformScale, Point posSphere, Point posSphere2, float sRadiusSphere)
{
	if (g_game.m_mpVkFJustPressed[VK_UP])
	{
		s_iDrawGjkDebug++;
	}
	else if (g_game.m_mpVkFJustPressed[VK_DOWN])
	{
		s_iDrawGjkDebug--;
	}

	SGjkBox gjkbox = SGjkBox(matCubePhys, vecNonuniformScale);
	SGjkIcosphere gjkico = SGjkIcosphere(posSphere, sRadiusSphere);
	Vector dPosSweep = posSphere2 - posSphere;

	{
		g_game.DebugDrawCube(MatScale(vecNonuniformScale) * matCubePhys);
		g_game.DebugDrawSphere(posSphere, sRadiusSphere);
		g_game.DebugDrawSphere(posSphere + dPosSweep, sRadiusSphere);

		TWEAKABLE int s_cGhostSphere = 10;
		for (int i = 0; i < s_cGhostSphere; i++)
		{
			g_game.DebugDrawSphere(posSphere + dPosSweep * ((i + 1) / float(s_cGhostSphere)), sRadiusSphere, 0.0f, SRgba(0.0f, 1.0f, 0.0f, 0.1f));
		}

		TWEAKABLE int s_cI = 10;
		TWEAKABLE int s_cJ = 10;
		g_game.DebugDrawSphere(s_posMinkowskiOriginDebugDraw, 0.5f, 0.0f, g_rgbaCyan);
		for (int i = 0; i < s_cI; i++)
		{
			for (int j = 0; j < s_cJ; j++)
			{
				float radJ = PI * j / float(s_cJ - 1.0f) - PI / 2.0f;
				Vector normal = VecCylind(TAU * i / float(s_cI), GCos(radJ), GSin(radJ));
				g_game.DebugDrawSphere(s_posMinkowskiOriginDebugDraw + MinkCompute(&gjkico, &gjkbox, dPosSweep, normal).m_posMink, 0.5f, 0.0f, SRgba(0.0f, 0.0f, 1.0f, 0.3f));
			}
		}
	}

	float s;
	bool fHit = FGjk(&gjkico, &gjkbox, dPosSweep, g_vecXAxis, true, &s);
	if (fHit)
	{
		g_game.PrintConsole("Hit!\n");
	}
	else
	{
		g_game.PrintConsole("Miss!\n");
	}
}

void TestGjk()
{
#if !SHIP
	TWEAKABLE bool s_fDebugGjk = ENABLE_DEBUG_GJK;

	if (!s_fDebugGjk)
		return;

	TWEAKABLE float s_sRadius = 1.0f;
	TWEAKABLE Point s_posSphere = Point(-1.0f, -5.0f, 5.0f);
	TWEAKABLE Point s_posSphere2 = Point(-1.0f, -5.0f, 5.0f);
	TWEAKABLE Point s_posBox = Point(0.0f, 0.0f, 5.0f);
	TWEAKABLE Vector s_vecScaleBox = Point(3.0f, 3.0f, 3.0f);
	TWEAKABLE float s_radRotatebox = 0.0f;
	TWEAKABLE Vector s_vecRotateBox = Vector(1.0f, 0.0f, 0.0f);

	s_posSphere = g_game.PosImgui(s_posSphere, { IMGUI(), 0});
	s_posSphere2 = g_game.PosImgui(s_posSphere2, { IMGUI(), 1});

	TestGjk(MatRotate(QuatAxisAngle(VecNormalize(s_vecRotateBox), s_radRotatebox)) * MatTranslate(s_posBox), s_vecScaleBox, s_posSphere, s_posSphere2, s_sRadius);
#endif
}
