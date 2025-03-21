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

Point PosSupportSweptIcosphere(Point posSphere, float sRadius, Vector dPosSweep, Vector normalSupport)
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
	for (int i = 0; i < 2; i++)
	{
		for (int iPos = 0; iPos < DIM(aPosIcosphere); iPos++)
		{
			Point pos = Vector(aPosIcosphere[iPos]) * sRadius + posSphere;
			if (i == 1)
			{
				pos += dPosSweep;
			}
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
#if 1
	// Implicit
	return PosSupportSweptSphere(posSphere, sRadius, dPosSweep, normalSupport) - PosSupportBox(matPhys, vecNonuniformScale, -normalSupport);
#else
	// Discrete
	return PosSupportSweptIcosphere(posSphere, sRadius, dPosSweep, normalSupport) - PosSupportBox(matPhys, vecNonuniformScale, -normalSupport);
#endif
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

bool FSimplex(Vector * pNormalSupport, SFixArray<Point, 4> * paryPosMink)
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

TWEAKABLE Point s_posMinkowskiOriginDebugDraw = Point(5.0f, 0.0f, 3.0f);
static int s_iDrawGjkDebug = 0;

void DoGjk(const Mat & matCubePhys, Vector vecNonuniformScale, Point posSphere, float sRadiusSphere, Vector dPosSweep, Vector normalSupport)
{
	SFixArray<Point, 4> aryPosMink;

	aryPosMink.Append(PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normalSupport));

	normalSupport = VecNormalizeSafe(-Vector(aryPosMink[0]));
	float sMin = FLT_MAX;

	SRgba aRgba[] = { g_rgbaRed, g_rgbaRed, g_rgbaOrange, g_rgbaYellow, g_rgbaPink };
	int iDraw = 0;

	int cIter = 0;

	bool fHit = false;
	TWEAKABLE int s_cIterMax = 30;

	g_game.PrintConsole(StrPrintf("iDraw:%i\n", s_iDrawGjkDebug));

	while (!fHit && cIter < s_cIterMax)
	{
		cIter++;
		Point posMinkNew = PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normalSupport);
		float gDotProgress = GDot(posMinkNew, normalSupport);
		if (gDotProgress < 0.0f)
			break;

		aryPosMink.Append(posMinkNew);

		Vector vecPosArrow = g_vecZero;
		SRgba rgba = SRgba(0,0,0,0);
		{
			if (iDraw == s_iDrawGjkDebug)
			{
				rgba = aRgba[aryPosMink.m_c];
				for (int i = 0; i < aryPosMink.m_c; i++)
				{
					vecPosArrow += aryPosMink[i] + s_posMinkowskiOriginDebugDraw;
					g_game.DebugDrawSphere(aryPosMink[i] + s_posMinkowskiOriginDebugDraw, .5f, 0.0f, rgba);
					for (int j = 0; j < aryPosMink.m_c; j++)
					{
						if (i == j)
							continue;
						g_game.DebugDrawLine(aryPosMink[i] + s_posMinkowskiOriginDebugDraw, aryPosMink[j] + s_posMinkowskiOriginDebugDraw, 0.0f, rgba);
					}
				}
				g_game.PrintConsole(StrPrintf("PointCount:%i\n", aryPosMink.m_c));
				vecPosArrow = vecPosArrow / aryPosMink.m_c;
			}
		}

		if (FSimplex(&normalSupport, &aryPosMink))
		{
			fHit = true;
		}

		{
			if (iDraw == s_iDrawGjkDebug)
			{
				ASSERT(FIsNear(1.0f, SLength(normalSupport)));
				g_game.DebugDrawArrow(Point(vecPosArrow), normalSupport * 3.0f, .1f, 0.0f, rgba);
			}
			iDraw++;
		}
	}

	if (fHit)
	{
		g_game.PrintConsole("HIT!\n");
	}
	else if (cIter == s_cIterMax)
	{
		g_game.PrintConsole("ITER OUT!\n");
	}
	else
	{
		g_game.PrintConsole("MISS!\n");
	}
}

void TestGjk(const Mat & matCubePhys, Vector vecNonuniformScale, Point posSphere, float sRadiusSphere)
{
	static float s_rSweep = 1.0f;//0.116382115f;//1.0f;
	if (g_game.m_mpVkFDown[VK_E])
	{
		s_rSweep += g_game.m_dT * .3f;
	}
	else if (g_game.m_mpVkFDown[VK_Q])
	{
		s_rSweep -= g_game.m_dT * .3f;
	}

	if (g_game.m_mpVkFJustPressed[VK_UP])
	{
		s_iDrawGjkDebug++;
	}
	else if (g_game.m_mpVkFJustPressed[VK_DOWN])
	{
		s_iDrawGjkDebug--;
	}

	TWEAKABLE Vector s_dPosSweep = Vector(5.0f, 5.0f, 5.0f);

	Vector dPosSweep = s_dPosSweep * s_rSweep;

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
				g_game.DebugDrawSphere(s_posMinkowskiOriginDebugDraw + PosMinkSweptSphereBox(posSphere, sRadiusSphere, dPosSweep, matCubePhys, vecNonuniformScale, normal), 0.5f, 0.0f, SRgba(0.0f, 0.0f, 1.0f, 0.3f));
				//DoGjk(matCubePhys, vecNonuniformScale, posSphere, sRadiusSphere, dPosSweep, normal); // TODO soak test
			}
		}
	}
	
	DoGjk(matCubePhys, vecNonuniformScale, posSphere, sRadiusSphere, dPosSweep, g_vecXAxis);
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









