#include "phys.h"
#include "engine.h"
#include <algorithm> // For sort

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
		float sRadiusLocal = sRadius / physcube.m_gUniformScale;

		// TODO first do a coarse check with a raycast against an inflated box w/the slab method

		// TODO handle moving physcube. Could basically say where was the sphere relative to us last frame, where does it want to end up relative to us this frame (even tho we've moved)

		// NOTE that instead of being in terms of time, this is ranges from 0-1 as a uDPos basically

		Point posOgLocal = posWorld * physcube.m_matPhysInverse;
		Vector vLocal = posNewLocal - posOgLocal;

		Vector vecTMin = VecComponentwiseDivide(- physcube.m_vecNonuniformScale - posOgLocal, vLocal);
		Vector vecTMax = VecComponentwiseDivide(physcube.m_vecNonuniformScale - posOgLocal, vLocal);

		// NOTE vLocal can be zero in any dimension

		float aT[8];
		int cT = 0;
		aT[cT++] = 0.0f;
		aT[cT++] = 1.0f;
		if (vLocal.X() != 0.0f)
		{
			aT[cT++] = vecTMin.X();
			aT[cT++] = vecTMax.X();
		}

		if (vLocal.Y() != 0.0f)
		{
			aT[cT++] = vecTMin.Y();
			aT[cT++] = vecTMax.Y();
		}

		if (vLocal.Z() != 0.0f)
		{
			aT[cT++] = vecTMin.Z();
			aT[cT++] = vecTMax.Z();
		}

		std::sort(aT, aT + cT);

		int iMic = -1;
		int iMac = -1;
		for (int i = 0; i < cT; i++)
		{
			if (aT[i] == 0.0f)
			{
				iMic = i;
			}
			else if (aT[i] == 1.0f)
			{
				iMac = i;
				break;
			}
		}

		bool fHit = false;
		float dTHit;
		for (int i = iMic; i < iMac; i++)
		{
			float dTPrev = aT[i];
			float dTNext = aT[i+1];

			struct STrack
			{
				float m_g; // fixed coordinate
				float m_v; // velocity
				float m_s; // origin
			};

			// NOTE this relies on the cube being symmetrical

			// BB janky way to figure out what quadrant we're in

			float dTMiddle = (dTPrev + dTNext) * 0.5f;

			Point posMiddle = posOgLocal + dTMiddle * vLocal;
			bool fTrackX = GAbs(posMiddle.X()) < physcube.m_vecNonuniformScale.m_vec.m_x;
			bool fTrackY = GAbs(posMiddle.Y()) < physcube.m_vecNonuniformScale.m_vec.m_y;
			bool fTrackZ = GAbs(posMiddle.Z()) < physcube.m_vecNonuniformScale.m_vec.m_z;

			STrack aTrack[3];
			int cTrack = 0;

			if (!fTrackX)
			{
				aTrack[cTrack].m_g = GSign(posMiddle.X()) * physcube.m_vecNonuniformScale.m_vec.m_x;
				aTrack[cTrack].m_v = vLocal.X();
				aTrack[cTrack].m_s = posOgLocal.X();
				cTrack++;
			}

			if (!fTrackY)
			{
				aTrack[cTrack].m_g = GSign(posMiddle.Y()) * physcube.m_vecNonuniformScale.m_vec.m_y;
				aTrack[cTrack].m_v = vLocal.Y();
				aTrack[cTrack].m_s = posOgLocal.Y();
				cTrack++;
			}

			if (!fTrackZ)
			{
				aTrack[cTrack].m_g = GSign(posMiddle.Z()) * physcube.m_vecNonuniformScale.m_vec.m_z;
				aTrack[cTrack].m_v = vLocal.Z();
				aTrack[cTrack].m_s = posOgLocal.Z();
				cTrack++;
			}

			// derived in https://www.wolframcloud.com/env/chasemacmillan/capsuleAABBintersection.nb

			// NOTE that tMin and tMax aren't necessarily tMin<tMax
			float tMin = -1.0f;
			float tMax = -1.0f;
			switch (cTrack)
			{
			case 0: // Completely inside
				// BB a bit of a guess
				ASSERT(false);
				tMin = dTPrev;
				tMax = dTNext;
				break;
			case 1: // Face
				{
					const STrack & trackA = aTrack[0];
					tMin = (trackA.m_g - sRadiusLocal - trackA.m_s) / trackA.m_v;
					tMax = (trackA.m_g + sRadiusLocal - trackA.m_s) / trackA.m_v;
				}
				break;
			case 2: // Edge
				{
					const STrack & trackA = aTrack[0];
					const STrack & trackB = aTrack[1];

					float a =	trackA.m_v * trackA.m_v +
								trackB.m_v * trackB.m_v;

					float b =	-2.0f * trackA.m_g * trackA.m_v + 2.0f * trackA.m_s * trackA.m_v
								-2.0f * trackB.m_g * trackB.m_v + 2.0f * trackB.m_s * trackB.m_v;

					float c =	trackA.m_g * trackA.m_g +
								trackB.m_g * trackB.m_g
								- sRadiusLocal * sRadiusLocal
								- 2.0f * trackA.m_g * trackA.m_s + trackA.m_s * trackA.m_s
								- 2.0f * trackB.m_g * trackB.m_s + trackB.m_s * trackB.m_s;
					float adT[2];
					int cdT = CSolveQuadratic(a, b, c, adT);
					if (cdT == 0)
						continue;
					else if (cdT == 1)
					{
						tMin = adT[0];
						tMax = adT[0];
					}
					else
					{
						tMin = adT[0];
						tMax = adT[1];
					}
				}
				break;
			case 3: // Corner
				{
					const STrack & trackA = aTrack[0];
					const STrack & trackB = aTrack[1];
					const STrack & trackC = aTrack[2];

					float a =	trackA.m_v * trackA.m_v +
								trackB.m_v * trackB.m_v + 
								trackC.m_v * trackC.m_v;

					float b =	-2.0f * trackA.m_g * trackA.m_v + 2.0f * trackA.m_s * trackA.m_v
								-2.0f * trackB.m_g * trackB.m_v + 2.0f * trackB.m_s * trackB.m_v
								-2.0f * trackC.m_g * trackC.m_v + 2.0f * trackC.m_s * trackC.m_v;

					float c =	trackA.m_g * trackA.m_g +
								trackB.m_g * trackB.m_g +
								trackC.m_g * trackC.m_g
								- sRadiusLocal * sRadiusLocal
								- 2.0f * trackA.m_g * trackA.m_s + trackA.m_s * trackA.m_s
								- 2.0f * trackB.m_g * trackB.m_s + trackB.m_s * trackB.m_s
								- 2.0f * trackC.m_g * trackC.m_s + trackC.m_s * trackC.m_s;
					float adT[2];
					int cdT = CSolveQuadratic(a, b, c, adT);
					if (cdT == 0)
						continue;
					else if (cdT == 1)
					{
						tMin = adT[0];
						tMax = adT[0];
					}
					else
					{
						tMin = adT[0];
						tMax = adT[1];
					}
				}
				break;
			default: // Should never happen
				ASSERT(false);
				continue;
			}

			if (GAbs(tMax) < 0.1f)
			{
				DoNothing();
			}

			float s_gEpsilon = .01f;

			if (FIsNear(tMin, 0.0f, s_gEpsilon) || FIsNear(tMax, 0.0f, s_gEpsilon))
			{
				fHit = true;
				dTHit = 0.0f;
				break;
			}
			else if (FIsNear(tMin, 1.0f, s_gEpsilon) || FIsNear(tMax, 1.0f, s_gEpsilon))
			{
				fHit = true;
				dTHit = 1.0f;
				break;
			}
			else if (0.0f <= tMin && tMin <= 1.0f)
			{
				fHit = true;
				dTHit = tMin;
				break;
			}
			else if (0.0f <= tMax && tMax <= 1.0f)
			{
				fHit = true;
				dTHit = tMax;
				break;
			}
		}

		if (!fHit)
			continue;

		posNew = (posOgLocal + vLocal * dTHit) * physcube.m_matPhys;	
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
