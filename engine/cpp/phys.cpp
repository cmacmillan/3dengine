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

Point PosSupportSweptSphere(Point posSphere, float sRadius, Vector dPosSweep, Vector normalSupport)
{
	float gDot = GDot(normalSupport, dPosSweep);
	Point posTest = (gDot > 0.0f) ? posSphere + dPosSweep : posSphere;
	return posTest + sRadius * normalSupport;
}

Point PosSupportBox(const Mat & matPhys, Vector vecNonuniformScale, Vector normalSupport)
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

Point PosMinkSweptSphereBox(Point posSphere, float sRadius, Vector dPosSweep, const Mat & matPhys, Vector vecNonuniformScale, Vector normalSupport)
{
	return PosSupportSweptSphere(posSphere, sRadius, dPosSweep, normalSupport) - PosSupportBox(matPhys, vecNonuniformScale, -normalSupport);
}

bool FHandleTriangle(Point pos0, Point pos1, Point pos2, Vector * pNormalOutward, Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
{
	// Handle optional "outward" vector used in tetrahedron case

	if (pNormalOutward && GDot(*pNormalOutward, -pos0) < 0.0f)
		return false;

	Vector normalTriangle = VecCross(pos1-pos0, pos2-pos0);
	Vector normal01 = VecCross(normalTriangle, pos1-pos0);
	Vector normal12 = VecCross(normalTriangle, pos2-pos1);
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
		paryPosMink->Empty();
		paryPosMink->Append(pos0);
		paryPosMink->Append(pos1);
		paryPosMink->Append(pos2);
	}

	return fInTri;
}

int IOriginInLineSegment(Point pos0, Point pos1)
{
	float gDot0 = GDot(-pos0, pos0 - pos1);
	float gDot1 = GDot(-pos0, pos0 - pos1);
	if (gDot0 < 0.0f && gDot1 < 0.0f)
	{
		// In segment
		return -1;
	}
	else if (gDot0 >= 0.0f)
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

void SetSimplexPoint(Point pos, Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
{
	paryPosMink->Empty();
	paryPosMink->Append(pos);
	*pNormalSupport = VecNormalizeSafe(-pos);
}

void SetSimplexLineSegment(Point pos0, Point pos1, Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
{
	paryPosMink->Empty();
	paryPosMink->Append(pos0);
	paryPosMink->Append(pos1);
	*pNormalSupport = VecNormalizeSafe(VecProjectOnTangent(-pos0, VecNormalizeSafe(pos1 - pos0)));
}

bool FSimplex2(Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
{
	switch (paryPosMink->m_c)
	{
	case 2:
		{
			Point pos0 = (*paryPosMink)[0];
			Point pos1 = (*paryPosMink)[1];
			switch (IOriginInLineSegment(pos0, pos1))
			{
				case -1:
					SetSimplexLineSegment(pos0, pos1, pNormalSupport, paryPosMink);
					break;

				case 0:
					SetSimplexPoint(pos0, pNormalSupport, paryPosMink);
					break;

				case 1:
					SetSimplexPoint(pos1, pNormalSupport, paryPosMink);
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
			Point pos0 = (*paryPosMink)[0];
			Point pos1 = (*paryPosMink)[1];
			Point pos2 = (*paryPosMink)[2];
			if (FHandleTriangle(pos0, pos1, pos2, nullptr, pNormalSupport, paryPosMink))
				return false;

			int iOrigin01 = IOriginInLineSegment(pos0, pos1);
			if (iOrigin01 == -1)
			{
				SetSimplexLineSegment(pos0, pos1, pNormalSupport, paryPosMink);
				return false;
			}

			int iOrigin12 = IOriginInLineSegment(pos1, pos2);
			if (iOrigin12 == -1)
			{
				SetSimplexLineSegment(pos1, pos2, pNormalSupport, paryPosMink);
				return false;
			}
			
			int iOrigin02 = IOriginInLineSegment(pos0, pos2);
			if (iOrigin02 == -1)
			{
				SetSimplexLineSegment(pos0, pos2, pNormalSupport, paryPosMink);
				return false;
			}

			if (iOrigin01 == 0 && iOrigin02 == 0)
			{
				SetSimplexPoint(pos0, pNormalSupport, paryPosMink);
				return false;
			}

			if (iOrigin01 == 1 && iOrigin12 == 0)
			{
				SetSimplexPoint(pos1, pNormalSupport, paryPosMink);
				return false;
			}

			SetSimplexPoint(pos2, pNormalSupport, paryPosMink);
			ASSERT(iOrigin02 == 1 && iOrigin12 == 1);
			return false;
		}
		break;
	case 4:
		{
			Point pos0 = (*paryPosMink)[0];
			Point pos1 = (*paryPosMink)[1];
			Point pos2 = (*paryPosMink)[2];
			Point pos3 = (*paryPosMink)[3];

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

			if (FHandleTriangle(pos0, pos1, pos2, &normal012, pNormalSupport, paryPosMink))
				return false;

			if (FHandleTriangle(pos0, pos1, pos3, &normal013, pNormalSupport, paryPosMink))
				return false;

			if (FHandleTriangle(pos1, pos2, pos3, &normal123, pNormalSupport, paryPosMink))
				return false;

			if (FHandleTriangle(pos0, pos2, pos3, &normal023, pNormalSupport, paryPosMink))
				return false;

			int iOrigin01 = IOriginInLineSegment(pos0, pos1);
			if (iOrigin01 == -1)
			{
				SetSimplexLineSegment(pos0, pos1, pNormalSupport, paryPosMink);
				return false;
			}

			int iOrigin12 = IOriginInLineSegment(pos1, pos2);
			if (iOrigin12 == -1)
			{
				SetSimplexLineSegment(pos1, pos2, pNormalSupport, paryPosMink);
				return false;
			}
			
			int iOrigin02 = IOriginInLineSegment(pos0, pos2);
			if (iOrigin02 == -1)
			{
				SetSimplexLineSegment(pos0, pos2, pNormalSupport, paryPosMink);
				return false;
			}

			int iOrigin03 = IOriginInLineSegment(pos0, pos3);
			if (iOrigin03 == -1)
			{
				SetSimplexLineSegment(pos0, pos3, pNormalSupport, paryPosMink);
				return false;
			}

			int iOrigin13 = IOriginInLineSegment(pos1, pos3);
			if (iOrigin13 == -1)
			{
				SetSimplexLineSegment(pos1, pos3, pNormalSupport, paryPosMink);
				return false;
			}
			
			int iOrigin23 = IOriginInLineSegment(pos2, pos3);
			if (iOrigin23 == -1)
			{
				SetSimplexLineSegment(pos2, pos3, pNormalSupport, paryPosMink);
				return false;
			}

			if (iOrigin01 == 0 && iOrigin02 == 0 && iOrigin03 == 0)
			{
				SetSimplexPoint(pos0, pNormalSupport, paryPosMink);
				return false;
			}

			if (iOrigin01 == 1 && iOrigin12 == 0 && iOrigin13 == 0)
			{
				SetSimplexPoint(pos1, pNormalSupport, paryPosMink);
				return false;
			}

			if (iOrigin02 == 1 && iOrigin12 == 1 && iOrigin23 == 0)
			{
				SetSimplexPoint(pos2, pNormalSupport, paryPosMink);
				return false;
			}

			ASSERT(iOrigin03 == 1 && iOrigin13 == 1 && iOrigin23 == 1);
			SetSimplexPoint(pos3, pNormalSupport, paryPosMink);
			return false;

		}
		break;

	default:
		ASSERT(false);
		return false;
	}
	return false;
}

bool FSimplex(Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
{
	// BB this is very janky and should be rewritten

	switch (paryPosMink->m_c)
	{
	case 2:
		{
			// I think we don't need to test the quadrant from pos0 in the direction of pos0-pos1

			Point pos0 = paryPosMink->m_a[0];
			Point pos1 = paryPosMink->m_a[1];

			Vector dPos = pos1 - pos0;
			Point posOriginRelToPos1 = -pos1;

			ASSERT(GDot(-pos0, dPos) > 0.0f);

			float gDot = GDot(posOriginRelToPos1, dPos);
			if (gDot > 0.0f)
			{
				paryPosMink->Empty();
				paryPosMink->Append(pos1);
				*pNormalSupport = VecNormalizeElse(posOriginRelToPos1, g_vecXAxis);
			}
			else
			{
				*pNormalSupport = VecNormalizeElse(VecProjectOnTangent(posOriginRelToPos1, VecNormalizeElse(dPos, g_vecXAxis)), g_vecXAxis);
			}
			return false;
		}
		break;

	case 3:
		{
			// Directions it can't be, pos0 in the direction of pos0-pos1,
			// or the pos0 pos1 edge

			Point pos0 = paryPosMink->m_a[0];
			Point pos1 = paryPosMink->m_a[1];
			Point pos2 = paryPosMink->m_a[2];

			Vector normalTriangle = VecNormalizeElse(VecCross(pos1 - pos0, pos2 - pos0), g_vecXAxis);
			Vector vecE = VecCross(normalTriangle, pos2 - pos1);
			Vector vecF = VecCross(normalTriangle, pos2 - pos0);

			// Ensure E and F point outward

			if (GDot(vecE, pos0 - pos2) > 0.0f)
			{
				vecE = -vecE;
			}

			if (GDot(vecF, pos1 - pos0) > 0.0f)
			{
				vecF = -vecF;
			}

			float gDotA = GDot(-pos1, pos1 - pos0);
			float gDotB = GDot(-pos1, pos1 - pos2);
			float gDotC = GDot(-pos2, pos2 - pos1);
			float gDotD = GDot(-pos2, pos2 - pos0);
			float gDotE = GDot(-pos2, vecE);
			float gDotF = GDot(-pos2, vecF);
			float gDotG = GDot(-pos0, pos0 - pos2);
			bool fPositiveA = gDotA > 0.0f;
			bool fPositiveB = gDotB > 0.0f;
			bool fPositiveC = gDotC > 0.0f;
			bool fPositiveD = gDotD > 0.0f;
			bool fPositiveE = gDotE > 0.0f;
			bool fPositiveF = gDotF > 0.0f;
			bool fPositiveG = gDotG > 0.0f;
			if (!fPositiveE && !fPositiveF)
			{
				// inside triangle
				float gDotTri = GDot(-pos0, normalTriangle);
				if (gDotTri > 0.0f)
				{
					// Along triangle normal

					*pNormalSupport = normalTriangle;
				}
				else
				{
					// Against triangle normal

					*pNormalSupport = -normalTriangle;
				}
			}
			else
			{
				if (fPositiveA && fPositiveB)
				{
					// Closest to pos1

					paryPosMink->Empty();
					paryPosMink->Append(pos1);
					*pNormalSupport = VecNormalizeElse(-pos1, g_vecXAxis);
				}
				else if (fPositiveC && fPositiveD)
				{
					// Closest to pos2

					paryPosMink->Empty();
					paryPosMink->Append(pos2);
					*pNormalSupport = VecNormalizeElse(-pos2, g_vecXAxis);
				}
				else if (!fPositiveB && !fPositiveC)
				{
					// Closest to E edge

					paryPosMink->Empty();
					paryPosMink->Append(pos1);
					paryPosMink->Append(pos2);
					*pNormalSupport = VecNormalizeElse(VecProjectOnTangent(-pos1, VecNormalizeElse(vecE, g_vecXAxis)), g_vecXAxis);
				}
				else if (!fPositiveD && !fPositiveG)
				{
					// Closest to F edge

					paryPosMink->Empty();
					paryPosMink->Append(pos0);
					paryPosMink->Append(pos2);
					*pNormalSupport = VecNormalizeElse(VecProjectOnTangent(-pos2, VecNormalizeElse(vecF, g_vecXAxis)), g_vecXAxis);
				}
				else
				{
					ASSERT(false);
				}
			}
			return false;
		}
		break;

	case 4:
		{
#if 0
			Point pos0 = paryPosMink->m_a[0];
			Point pos1 = paryPosMink->m_a[1];
			Point pos2 = paryPosMink->m_a[2];
			Point pos3 = paryPosMink->m_a[3];

			// All normals point out

			Vector normalTriangle013 = VecNormalizeElse(VecCross(pos1 - pos0, pos3 - pos0), g_vecXAxis);
			Vector normalTriangle123 = VecNormalizeElse(VecCross(pos2 - pos1, pos3 - pos1), g_vecXAxis);
			Vector normalTriangle023 = VecNormalizeElse(VecCross(pos2 - pos0, pos3 - pos0), g_vecXAxis);

			// Ensure triangle normals point out

			if (GDot(normalTriangle013, pos2 - pos0) > 0.0f)
			{
				normalTriangle013 = -normalTriangle013;
			}

			if (GDot(normalTriangle123, pos0 - pos1) > 0.0f)
			{
				normalTriangle123 = -normalTriangle123;
			}

			if (GDot(normalTriangle023, pos1 - pos0) > 0.0f)
			{
				normalTriangle023 = -normalTriangle023;
			}

			float gDot013 = GDot(-pos0, normalTriangle013);
			float gDot123 = GDot(-pos1, normalTriangle123);
			float gDot023 = GDot(-pos0, normalTriangle023);

			if (gDot013 < 0.0f && gDot123 < 0.0f && gDot023 < 0.0f)
			{
				// Inside tetrahedron
				return true;
			}
			else
			{
				// No need to test 012, since we know the origin is greater than it

				// 6 edge normals

				// Test each corner

				// BB doing unneeded tests here

				float gDotA = GDot(-pos0, pos0 - pos1);
				float gDotB = GDot(-pos0, pos0 - pos2);
				float gDotC = GDot(-pos0, pos0 - pos3);
				bool fPositiveA = gDotA > 0.0f;
				bool fPositiveB = gDotB > 0.0f;
				bool fPositiveC = gDotC > 0.0f;

				float gDotD = GDot(-pos1, pos1 - pos0);
				float gDotE = GDot(-pos1, pos1 - pos2);
				float gDotF = GDot(-pos1, pos1 - pos3);
				bool fPositiveD = gDotD > 0.0f;
				bool fPositiveE = gDotE > 0.0f;
				bool fPositiveF = gDotF > 0.0f;

				float gDotG = GDot(-pos2, pos2 - pos0);
				float gDotH = GDot(-pos2, pos2 - pos1);
				float gDotI = GDot(-pos2, pos2 - pos3);
				bool fPositiveG = gDotG > 0.0f;
				bool fPositiveH = gDotH > 0.0f;
				bool fPositiveI = gDotI > 0.0f;

				float gDotJ = GDot(-pos3, pos3 - pos0);
				float gDotK = GDot(-pos3, pos3 - pos1);
				float gDotL = GDot(-pos3, pos3 - pos2);
				bool fPositiveJ = gDotJ > 0.0f;
				bool fPositiveK = gDotK > 0.0f;
				bool fPositiveL = gDotL > 0.0f;


				if (fPositiveA && fPositiveB && fPositiveC)
				{
					// near pos 0
				}
				else if (fPositiveD && fPositiveE && fPositiveF)
				{
					// near pos 1
				}
				else if (fPositiveG && fPositiveH && fPositiveI)
				{
					// near pos 2
				}
				else if (fPositiveJ && fPositiveK && fPositiveL)
				{
					// near pos 3
				}
				/*
				if (gDot013 > 0.0f)
				{
					
				}
				else if ()
				{

				}
				else
				{
					ASSERT(false);
				}
				*/
			}
#endif
		}
		break;

	default:
		ASSERT(false);
		return false;
	}
	return true;
}

void TestGjk(const Mat & matCubePhys, Vector vecNonuniformScale, Point posSphere, float sRadiusSphere)
{
	TWEAKABLE Vector s_dPosSweep = Vector(5.0f, 5.0f, 5.0f);
	TWEAKABLE Point s_posMinkowskiOrigin = Point(5.0f, 0.0f, 3.0f);

	static float s_rSweep = 1.0f;
	if (g_game.m_mpVkFDown[VK_E])
	{
		s_rSweep += g_game.m_dT * .3f;
	}
	else if (g_game.m_mpVkFDown[VK_Q])
	{
		s_rSweep -= g_game.m_dT * .3f;
	}

	Vector dPosSweep = s_dPosSweep * s_rSweep;

	{
		g_game.DebugDrawCube(MatScale(vecNonuniformScale) * matCubePhys);
		g_game.DebugDrawSphere(posSphere, sRadiusSphere);
		g_game.DebugDrawSphere(posSphere + dPosSweep, sRadiusSphere);

#if 0
		{
			Vector normalSupport;
			{
				TWEAKABLE Vector s_vecSupport = Vector(.3f, -.2f, .1f);
				normalSupport = VecNormalize(s_vecSupport);
			}
			g_game.DebugDrawArrow(matCubePhys.m_aVec[3], normalSupport * 4.0f, 0.1f, 0.0f, g_rgbaRed);
			g_game.DebugDrawSphere(PosSupportBox(matCubePhys, vecNonuniformScale, normalSupport), 0.25f, 0.0f, g_rgbaRed);
			g_game.DebugDrawSphere(PosSupportSweptSphere(posSphere, sRadiusSphere, dPosSweep, normalSupport), 0.25f, 0.0f, g_rgbaRed);
		}
#endif

		TWEAKABLE int s_cGhostSphere = 10;
		for (int i = 0; i < s_cGhostSphere; i++)
		{
			g_game.DebugDrawSphere(posSphere + dPosSweep * ((i + 1) / float(s_cGhostSphere)), sRadiusSphere, 0.0f, SRgba(0.0f, 1.0f, 0.0f, 0.1f));
		}

		TWEAKABLE int s_cI = 10;
		TWEAKABLE int s_cJ = 10;
		g_game.DebugDrawSphere(s_posMinkowskiOrigin, 0.5f, 0.0f, g_rgbaCyan);
		for (int i = 0; i < s_cI; i++)
		{
			for (int j = 0; j < s_cJ; j++)
			{
				float radJ = PI * j / float(s_cJ - 1.0f) - PI / 2.0f;
				Vector normal = VecCylind(TAU * i / float(s_cI), GCos(radJ), GSin(radJ));
				g_game.DebugDrawSphere(s_posMinkowskiOrigin + PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normal), 0.5f, 0.0f, SRgba(0.0f, 0.0f, 1.0f, 0.3f));
			}
		}
	}

	//// Actual algo

	{
		Vector normalSupport = g_vecXAxis;

		SFixArray<Point, 4> aryPosMink;

		aryPosMink.Append(PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normalSupport));

		normalSupport = VecNormalizeElse(-Vector(aryPosMink[0]), g_vecXAxis);
		float sMin = FLT_MAX;

		SRgba aRgba[] = { g_rgbaRed, g_rgbaOrange, g_rgbaYellow, g_rgbaPink };
		float aRadius[] = { .5f, .4f,.3f, .2f };
		int iRgba = 0;
		static int s_iDraw = 0;
		int iDraw = 0;
		if (g_game.m_mpVkFJustPressed[VK_UP])
		{
			s_iDraw++;
		}
		else if (g_game.m_mpVkFJustPressed[VK_DOWN])
		{
			s_iDraw--;
		}

		bool fHit = false;
		while (!fHit)
		{
			Point posMinkNew = PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normalSupport);
			if (GDot(posMinkNew, normalSupport) < 0.0f)
				break;

			aryPosMink.Append(posMinkNew);

			Vector vecPosArrow = g_vecZero;
			{
				if (iDraw == s_iDraw)
				{
					for (int i = 0; i < aryPosMink.m_c; i++)
					{
						vecPosArrow += aryPosMink[i] + s_posMinkowskiOrigin;
						g_game.DebugDrawSphere(aryPosMink[i] + s_posMinkowskiOrigin, aRadius[iRgba], 0.0f, aRgba[iRgba]);
					}
					vecPosArrow = vecPosArrow / aryPosMink.m_c;
				}
			}
			
			if (FSimplex2(&normalSupport, &aryPosMink))
			{
				fHit = true;
			}

			{
				if (iDraw == s_iDraw)
				{
					ASSERT(FIsNear(1.0f, SLength(normalSupport)));
					g_game.DebugDrawArrow(Point(vecPosArrow), normalSupport * 3.0f, .1f, 0.0f, aRgba[iRgba]);
				}
				iRgba++;
				if (iRgba >= DIM(aRgba))
				{
					iRgba = 0;
				}
				iDraw++;
			}
		}
	}
}

void TestGjk()
{
	TWEAKABLE bool s_fDebug = true;

	if (!s_fDebug)
		return;

#if 0
	TWEAKABLE int s_iPhyscubeTest = 0;
	TWEAKABLE int s_iDynsphereTest = 0;
	auto & aryPhyscube = g_objman.m_mpTypekAryPObj[TYPEK_PhysCube];
	auto & aryDynsphere = g_objman.m_mpTypekAryPObj[TYPEK_DynSphere];
	SDynSphere * pDynsphere = (SDynSphere *) aryDynsphere[s_iDynsphereTest];
	SPhysCube * pPhyscube = (SPhysCube *)aryPhyscube[s_iPhyscubeTest];
	TestGjk(pPhyscube->m_matPhys, pPhyscube->m_vecNonuniformScale, pDynsphere->PosWorld(), pDynsphere->SRadius());
#endif

	TWEAKABLE float s_sRadius = 1.0f;
	TWEAKABLE Point s_posSphere = Point(-1.0f, -5.0f, 5.0f);
	TWEAKABLE Point s_posBox = Point(0.0f, 0.0f, 5.0f);
	TWEAKABLE Vector s_vecScaleBox = Point(3.0f, 3.0f, 3.0f);
	TWEAKABLE float s_radRotatebox = 0.0f;
	TWEAKABLE Vector s_vecRotateBox = Vector(1.0f, 0.0f, 0.0f);

	TestGjk(MatRotate(QuatAxisAngle(VecNormalize(s_vecRotateBox), s_radRotatebox)) * MatTranslate(s_posBox), s_vecScaleBox, s_posSphere, s_sRadius);
}









